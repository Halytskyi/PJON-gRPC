// Common libraries
#include "version.h"
#include <stdio.h>
#include <thread>
#include <queue>
#include <vector>
// PJON library
#define TS_RESPONSE_TIME_OUT 35000
#define PJON_INCLUDE_TS true // Include only ThroughSerial
#ifndef RPI
  #define RPI true
#endif
#include "PJON/PJON.h"
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
// RPI serial interface
#include <wiringPi.h>
#include <wiringSerial.h>
// gRPC library
#include <iostream>
#include <memory>
#include <grpc++/grpc++.h>
#include "pjongrpc.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using pjongrpc::Arduino_Request;
using pjongrpc::Arduino_Reply;
using pjongrpc::Arduino;

// TODO: Find the way to change in from command line
PJON<ThroughSerial> bus(1);

int debug = 0;
uint64_t rcvd_cnt = 0;
int client_query = 0;
std::string client_response = "";
std::string response;
std::vector<std::string> receives_array = {"", ""};
std::queue<std::vector<std::string>> receives_queue;


static void receiver_function(
        uint8_t *payload,
        uint16_t length,
        const PJON_Packet_Info &packet_info){

  response = "";
  for (uint32_t i = 0; i != length; i++){
    response += payload[i];
  }

  if (debug == 1) {
    rcvd_cnt += 1;
    std::cout << "#RCV snd_id=" << std::to_string(packet_info.sender_id)
              << " snd_net=";
                            for (uint32_t i = 0; i < sizeof(packet_info.sender_bus_id); i++) {
                              std::cout << std::to_string(packet_info.sender_bus_id[i]);
                              if (i < sizeof(packet_info.sender_bus_id) - 1)
                                std::cout << ".";
                            }
    std::cout << " rcv_id=" << std::to_string(packet_info.receiver_id)
              << " rcv_net=";
                            for (uint32_t i = 0; i < sizeof(packet_info.receiver_bus_id); i++) {
                              std::cout << std::to_string(packet_info.receiver_bus_id[i]);
                              if (i < sizeof(packet_info.receiver_bus_id) - 1)
                                std::cout << ".";
                            }
    std::cout << " id=" << std::to_string(packet_info.id)
              << " hdr=" << std::to_string(packet_info.header)
              << " pckt_cnt=" << std::to_string(rcvd_cnt)
              << " len=" << length
              << " data=" << response;
    std::cout << std::endl;
  }

  // TODO: need implement additional check that we waiting exactly for this response, should be possible in PJON v10
  if (client_query == packet_info.sender_id) {
    client_response = response;
  } else {
    receives_array[0] = std::to_string(packet_info.sender_id);
    receives_array[1] = response;
    receives_queue.push(receives_array);
  }
};

static void error_handler_function(uint8_t code, uint8_t data) {
  std::cout << "#ERR code=" << std::to_string(code);
  std::cout << " data=" << std::to_string(data);
  std::cout << std::endl;
};

bool is_enough_args(int argc, char **argv) {
  if (argc < 4)
    return false;
  return true;
}

bool is_first_arg_com_port(int argc, char **argv) {
  if (std::string(argv[1]).find("tty") != std::string::npos)
    return true;
  return false;
}

bool is_second_arg_bitrate(int argc, char **argv) {
  if (0 < std::stoi(std::string(argv[2])))
    if(std::stoi(std::string(argv[2]))  <= 153600)
      return true;
  return false;
}

bool is_third_arg_bus_id(int argc, char **argv) {
  if (0 <= std::stoi(std::string(argv[3])))
    if (std::stoi(std::string(argv[3])) <= 255)
      return true;
  return false;
}

bool is_fourth_arg_debug(int argc, char **argv) {
  if (argv[4])
    if (std::string(argv[4]) == "debug")
      return true;
  return false;
}

void print_usage_help() {
  std::cout
      << "PJON_gRPC - gRPC server-client for PJON bus\n"
      << "VERSION: " << PJON_gRPC_SERVER_VERSION << "\n"
      << "\n"
      << "usage: pjon_grpc_server <COM PORT> <BITRATE> <NODE ID> <debug>\n"
      << "                           \\          \\         \\         \\\n"
      << "                            \\          \\       0-255   optional parameter\n"
      << "                     /dev/ttyXXXX     1200 - 153600\n"
      << std::endl
      << "example: pjon_grpc_server /dev/ttyUSB0 115200 1" << std::endl
      << std::endl
      << "other options:" << std::endl
      << "   help - print this help" << std::endl
      << "version - displays program version" << std::endl
      << "--------------------------------------" << std::endl
      ;
}

void pjon_communication(int node_id, const char* data) {
  if (debug == 1) {
    printf("Received command: %s\n", data);
    printf("Attempting to send a packet... \n");
  }
  bus.send(node_id, data, strlen(data));
  if (debug == 1)
    printf("Attempting to roll bus... \n");
  bus.update();
}

class ArduinoServiceImpl final : public Arduino::Service {
  Status RPiArduino(ServerContext* context, const Arduino_Request* request,
                  Arduino_Reply* reply) override {
    
    int node_id = request->node_id();
    const char* data = request->data().c_str();
    client_query = node_id;
    pjon_communication(node_id, data);
    uint32_t time = micros();
    while(micros() - time < 3000000) {
      if (client_response != "") {
        time = micros() - 3000000;
      }
    }
    reply->set_message(client_response);
    client_query = 0;
    client_response = "";
    return Status::OK;
  }
};

void run_server() {
  std::string server_address("0.0.0.0:50051");
  ArduinoServiceImpl service;
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  if (debug == 1)
    std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

class ArduinoClient {
 public:
  ArduinoClient(std::shared_ptr<Channel> channel)
      : stub_(Arduino::NewStub(channel)) {}
  std::string RPiArduino(int node_id, const std::string& data) {
    Arduino_Request request;
    request.set_node_id(node_id);
    request.set_data(data);
    Arduino_Reply reply;
    ClientContext context;
    Status status = stub_->RPiArduino(&context, request, &reply);
    if (status.ok()) {
      return reply.message();
    } else {
      if (debug == 1) {
        std::cout << status.error_code() << ": " << status.error_message()
                  << std::endl;
      }
      return "RPC failed";
    }
  }
 private:
  std::unique_ptr<Arduino::Stub> stub_;
};

void grpc_client() {
  ArduinoClient arduino(grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials()));
  while(true) {
    if (receives_queue.size() != 0) {
      while(!receives_queue.empty()) {
        int node_id = stoi(receives_queue.front()[0]);
        std::string data = receives_queue.front()[1];
        std::string reply = arduino.RPiArduino(node_id, data);
        if (debug == 1)
          std::cout << "Arduino answered: " << reply << std::endl;
        if (reply == "done")
          receives_queue.pop();
        delayMicroseconds(1000000);
      }
    }
    delayMicroseconds(1000000);
  }
}

void listen_on_bus() {
  while(true) {
    bus.receive(1000);
    delayMicroseconds(500000);
  }
}

int main(int argc, char** argv) {
  if (argc == 2) {
    if (std::string(argv[1]) == "help") {
      print_usage_help();
      return 0;
    } else if (std::string(argv[1]) == "version") {
      std::cout << "VERSION: " << PJON_gRPC_SERVER_VERSION << "\n";
      return 0;
    }
    print_usage_help();
    std::cerr << "ERROR: option not supported\n";
    return 1;
  }
  if (!is_enough_args(argc, argv)) {
    print_usage_help();
    std::cerr << "ERROR: not enough args\n";
    return 1;
  }
  if (!is_first_arg_com_port(argc, argv)) {
    print_usage_help();
    std::cerr << "ERROR: first arg <COM PORT> should be /dev/ttyXXXX\n";
    return 1;
  }
  if (!is_second_arg_bitrate(argc, argv)) {
    print_usage_help();
    std::cerr << "ERROR: second arg <BITRATE> should specify bitrate 1 - 153600 like 2400, 19200, 38400, 57600, 115200, 153600\n";
    return 1;
  }
  if (!is_third_arg_bus_id(argc, argv)) {
    print_usage_help();
    std::cerr << "ERROR: third arg <NODE ID> should specify bus address 0 - 255\n";
    return 1;
  }
  if (is_fourth_arg_debug(argc, argv)) {
    debug = 1;
  }

  char* com_str = argv[1];
  int bitRate = std::stoi(std::string(argv[2]));
// TODO: How we can define it globally?
//  PJON<ThroughSerial> bus(std::stoi(std::string(argv[3])));

  try {
    if (debug == 1)
      printf("Opening serial... \n");
    int s = serialOpen(com_str, bitRate);
    if (int(s) < 0) {
      printf("Serial open fail!\n");
      exit (EXIT_FAILURE);
    }
    if (wiringPiSetup() == -1) {
      printf("WiringPi setup fail");
      exit (EXIT_FAILURE);
    }

    if (debug == 1)
      printf("Setting serial... \n");
    bus.strategy.set_serial(s);
    bus.strategy.set_baud_rate(bitRate);

    if (debug == 1)
      printf("Opening bus... \n");
    bus.begin();
    bus.set_receiver(receiver_function);
    if (debug == 1)
      bus.set_error(error_handler_function);
    // with enabled ask we have repeated requests send time to time
    bus.set_synchronous_acknowledge(false);
    // crc_8 doesn't work correctly with 8.x PJON version, can be fixed in v9.x
    bus.set_crc_32(true);

    std::thread listen_on_bus_thd(listen_on_bus);
    std::thread grpc_client_thd(grpc_client);

    run_server();
    return 0;
  }
  catch (const char* msg) {
    std::cout << "exc: "
              << msg
              << std::endl;
    return 1;
  }

}
