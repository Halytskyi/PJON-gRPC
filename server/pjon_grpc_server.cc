// Common libraries
#include "version.h"
#include <stdio.h>
// PJON library
#define TS_RESPONSE_TIME_OUT 75000
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
using grpc::Status;
using pjongrpc::Arduino_Request;
using pjongrpc::Arduino_Reply;
using pjongrpc::Arduino;

// TODO: Find the way to change in from command line
PJON<ThroughSerial> bus(1);

uint64_t rcvd_cnt = 0;
std::string response;
int send_tries = 5;


static void receiver_function(
        uint8_t *payload,
        uint16_t length,
        const PJON_Packet_Info &packet_info){

  for (uint32_t i = 0; i != length; i++){
    response += payload[i];
  }

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

void print_usage_help() {
  std::cout
      << "PJON_gRPC - gRPC server-client for PJON bus\n"
      << "VERSION: " << PJON_gRPC_SERVER_VERSION << "\n"
      << "\n"
      << "usage: pjon_grpc_server <COM PORT> <BITRATE> <NODE ID>\n"
      << "                           \\          \\         \\\n"
      << "                            \\          \\       0-255\n"
      << "                     /dev/ttyXXXX     1200 - 153600\n"
      << std::endl
      << "example: pjon_grpc_server /dev/ttyUSB0 57600 1" << std::endl
      << std::endl
      << "other options:" << std::endl
      << "   help - print this help" << std::endl
      << "version - displays program version" << std::endl
      << "--------------------------------------" << std::endl
      ;
}

void pjon_communication(int node_id, const char* data) {
  printf(data);
  printf("\n");
  printf("Attempting to send a packet... \n");
  bus.send(node_id, data, strlen(data));
  printf("Attempting to roll bus... \n");
  bus.update();
  printf("Attempting to receive from bus... \n");
  bus.receive(1000000);
/*  uint32_t time = micros();
  while(micros() - time < 1000000) {
    bus.update();
    bus.receive();
  } */
}

class ArduinoServiceImpl final : public Arduino::Service {
  Status RPiArduino(ServerContext* context, const Arduino_Request* request,
                  Arduino_Reply* reply) override {
    response = "";
    int node_id = request->node_id();
    const char* data = request->data().c_str();
    pjon_communication(node_id, data);
    // Retry send request if didn't get data
    if (response == "") {
      int send_try = 1;
      while(send_try != send_tries +1) {
        pjon_communication(node_id, data);
        printf("Repeat: %d\n", send_try);
        if (response == "") {
          send_try += 1;
        } else {
          send_try = send_tries + 1;
        }
      }
    }
    reply->set_message(response);
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
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
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

  char* com_str = argv[1];
  int bitRate = std::stoi(std::string(argv[2]));
// TODO: How we can define it globally?
//  PJON<ThroughSerial> bus(std::stoi(std::string(argv[3])));

  try {
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

    printf("Setting serial... \n");
    bus.strategy.set_serial(s);
    bus.strategy.set_baud_rate(bitRate);

    printf("Opening bus... \n");
    bus.begin();
    bus.set_receiver(receiver_function);
    bus.set_error(error_handler_function);
    // with enabled ask we have repeated requests send time to time
    bus.set_synchronous_acknowledge(false);
    // crc_8 doesn't work correctly with 8.x PJON version, can be fixed in v9.x
    bus.set_crc_32(true);

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
