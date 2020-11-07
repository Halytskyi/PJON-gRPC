// Common libraries
#include <INIReader.h>
#include "version.h"
#include <stdio.h>
#include <thread>
#include <queue>
#include <vector>
#include <ctime>
// PJON library
#ifndef RPI
  #define RPI true
#endif
#define PJON_INCLUDE_PACKET_ID
#define TS_RESPONSE_TIME_OUT 150000
#include <PJONThroughSerial.h>
// RPI serial interface
#include <wiringPi.h>
#include <wiringSerial.h>
// gRPC library
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

// Get values from configuration file
std::string config_file = "conf/pjon-grpc.cfg";
INIReader reader(config_file);
int master_id = reader.GetInteger("main", "master-id", 1);
bool debug = reader.GetBoolean("main", "debug", false);
bool receiver = reader.GetBoolean("main", "receiver", false);
int retry_requests = reader.GetInteger("main", "retry-requests", 0);
std::string delimiter_transmit = reader.Get("main", "delimiter-transmit", ">");
std::string error_name = reader.Get("main", "error-name", "err");
bool transmitter = reader.GetBoolean("main", "transmitter", false);
std::string delimiter_receive = reader.Get("main", "delimiter-receive", "<");

PJONThroughSerial bus(master_id);

uint64_t rcvd_cnt = 0;
std::vector<std::array<std::string, 3>> check_messages_array;
std::vector<std::string> receives_array = {"", ""};
std::queue<std::vector<std::string>> receives_queue;


void message_processing(std::string response, int packet_sender_id) {
  if (receiver and response.find(delimiter_transmit) != std::string::npos) {
    check_messages_array.push_back({std::to_string(packet_sender_id), response, std::to_string(time(0))});
  } else if (transmitter and response.find(delimiter_receive) != std::string::npos) {
    receives_array[0] = std::to_string(packet_sender_id);
    receives_array[1] = response;
    receives_queue.push(receives_array);
  }
}

static void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  std::string response = "";
  for (uint32_t i = 0; i != length; i++){
    response += payload[i];
  }
  if (debug) {
    rcvd_cnt += 1;
    std::cout << "#RCV snd_id=" << std::to_string(packet_info.tx.id)
              << " snd_net=";
                            for (uint32_t i = 0; i < sizeof(packet_info.tx.bus_id); i++) {
                              std::cout << std::to_string(packet_info.tx.bus_id[i]);
                              if (i < sizeof(packet_info.tx.bus_id) - 1)
                                std::cout << ".";
                            }
    std::cout << " rcv_id=" << std::to_string(packet_info.rx.id)
              << " rcv_net=";
                            for (uint32_t i = 0; i < sizeof(packet_info.rx.bus_id); i++) {
                              std::cout << std::to_string(packet_info.rx.bus_id[i]);
                              if (i < sizeof(packet_info.rx.bus_id) - 1)
                                std::cout << ".";
                            }
    std::cout << " id=" << std::to_string(packet_info.id)
              << " hdr=" << std::to_string(packet_info.header)
              << " pckt_cnt=" << std::to_string(rcvd_cnt)
              << " len=" << length
              << " data=" << response;
    std::cout << std::endl;
  }
  message_processing(response, packet_info.tx.id);
};

static void error_handler(uint8_t code, uint16_t data, void *custom_pointer) {
  if(code == PJON_CONNECTION_LOST) {
    std::string error_device;
    error_device = std::to_string(bus.packets[data].content[0]);
    std::cout << "Connection with device ID " << error_device << " is lost." << std::endl;
  }
  if(code == PJON_PACKETS_BUFFER_FULL) {
    std::cout << "#ERR code=" << std::to_string(code) << std::endl;
    std::cout << "Packet buffer is full, has now a length of " << std::to_string(data) << std::endl;
    std::cout << "Possible wrong bus configuration!" << std::endl;
    std::cout << "higher PJON_MAX_PACKETS if necessary." << std::endl;
  }
  if(code == PJON_CONTENT_TOO_LONG) {
    std::cout << "Content is too long, length: " << std::to_string(data) << std::endl;
  }
};

// Server
void run_server(std::string bind_ip) {
  class ArduinoServiceImpl final : public Arduino::Service {
    Status RPiArduino(ServerContext* context, const Arduino_Request* request,
                    Arduino_Reply* reply) override {
      int node_id = request->node_id();
      const char* data = request->data().c_str();
      std::string response;
      int send_requests = retry_requests + 1;
      while(send_requests != 0) {
        if (debug) {
          std::cout << "Received command: " << data << std::endl;
          std::cout << "Attempting to send a packet..." << std::endl;
        }
        bus.send_packet_blocking(node_id, data, strlen(data));
        if (debug)
          std::cout << "Attempting to roll bus..." << std::endl;
        uint32_t time_start = micros();
        while(micros() - time_start < 1000000) {
          if (check_messages_array.size() != 0) {
            for (int i=0; i<check_messages_array.size(); ++i) {
              std::string response_data = check_messages_array[i][1].substr(0, check_messages_array[i][1].find(delimiter_transmit));
              if (time(0) - 3 >= std::stoi(check_messages_array[i][2])) {
                check_messages_array.erase(check_messages_array.begin()+i);
                i -= 1;
              } else if (check_messages_array[i][0] == std::to_string(node_id) and response_data == data) {
                response = check_messages_array[i][1];
                check_messages_array.erase(check_messages_array.begin()+i);
                time_start = micros() - 1000000;
                send_requests = 1;
                break;
              } else if (check_messages_array[i][0] == std::to_string(node_id) and response_data == error_name) {
                response = check_messages_array[i][1];
                check_messages_array.erase(check_messages_array.begin()+i);
                time_start = micros() - 1000000;
                send_requests = 1;
                break;
              }
            }
          }
        }
        send_requests -= 1;
      }
      if (debug) {
        std::cout << "Client response: " << response << std::endl;
        std::cout << "Array size: " << check_messages_array.size() << std::endl << std::endl;
      }
      reply->set_message(response);
      return Status::OK;
    }
  };

  std::string server_address(bind_ip);
  ArduinoServiceImpl service;
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

void grpc_client(std::string grpc_server_ip) {
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
        if (debug) {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
        }
        return "RPC failed";
      }
    }
   private:
    std::unique_ptr<Arduino::Stub> stub_;
  };
  std::cout << "gRPC client initialized and ready transmit data to " << grpc_server_ip << std::endl;

  ArduinoClient arduino(grpc::CreateChannel(grpc_server_ip, grpc::InsecureChannelCredentials()));
  while(true) {
    if (receives_queue.size() != 0) {
      while(!receives_queue.empty()) {
        int node_id = stoi(receives_queue.front()[0]);
        std::string data = receives_queue.front()[1];
        std::string reply = arduino.RPiArduino(node_id, data);
        if (debug)
          std::cout << "gRPC server answer: " << reply << std::endl;
        if (reply == "done")
          receives_queue.pop();
        delayMicroseconds(100000);
      }
    }
    delayMicroseconds(100000);
  }
}

void listen_on_bus() {
  while(true) {
    bus.receive();
    delayMicroseconds(1);
  }
}

void print_usage_help() {
  std::cout
    << "PJON_gRPC - gRPC server-client for PJON bus" << std::endl
    << "VERSION: " << PJON_gRPC_SERVER_VERSION << std::endl
    << std::endl
    << "Options:"<< std::endl
    << "-h, help - print this help" << std::endl
    << "-v, version - displays program version" << std::endl
    << std::endl;
}

int main(int argc, char** argv) {
  if (argc == 2) {
    if (std::string(argv[1]) == "-h" or std::string(argv[1]) == "help") {
      print_usage_help();
      return 0;
    } else if (std::string(argv[1]) == "-v" or std::string(argv[1]) == "version") {
      std::cout << "VERSION: " << PJON_gRPC_SERVER_VERSION << "\n";
      return 0;
    } else {
      print_usage_help();
      std::cerr << "ERROR: option not supported\n";
      return 1;
    }
  } else if (argc != 1) {
    print_usage_help();
    std::cerr << "ERROR: option not supported\n";
    return 1;
  }

  if (reader.ParseError() < 0) {
    std::cout << "Can't load config file: " << config_file << std::endl;
    return 1;
  }

  const char* serial_device = reader.Get("main", "serial-device", "/dev/ttyUSB0").c_str();
  int bitrate = reader.GetInteger("main", "bitrate", 9600);
  std::string bind_ip = reader.Get("main", "bind-ip", "0.0.0.0:50051");
  std::string grpc_server_ip = reader.Get("main", "grpc-server-ip", "127.0.0.1:50061");

  if (std::string(serial_device).find("/dev/tty") == std::string::npos) {
    std::cout << "Wrong value for 'serial-device' parameter: '" << serial_device <<"', should be '/dev/ttyXXXX'\n";
    return 1;
  } else if (bitrate < 2400 or bitrate > 153600) {
    std::cout << "Wrong value for 'bitrate' parameter: '" << bitrate <<"', should specify bitrate '1 - 153600', like: 2400, 9600, 19200, 38400, 57600, 115200, 153600\n";
    return 1;
  }

  if (!transmitter and !receiver) {
    std::cout << "'transmitter' and 'receiver' disabled. You should enable at least one of them" << std::endl;
    return 1;
  }

  try {
    if (wiringPiSetup() == -1) {
      std::cout << "WiringPi setup fail" << std::endl;
      exit (EXIT_FAILURE);
    }

    if (debug)
      std::cout << "Opening serial..." << std::endl;
    int s = serialOpen(serial_device, bitrate);
    if (int(s) < 0) {
      std::cout << "Serial open fail!" << std::endl;
      exit (EXIT_FAILURE);
    }
    if (debug)
      std::cout << "Setting serial..." << std::endl;
    bus.strategy.set_serial(s);
    bus.strategy.set_baud_rate(bitrate);
    if (debug)
      std::cout << "Opening bus..." << std::endl;
    bus.begin();
    bus.set_receiver(receiver_function);
    if (debug)
      bus.set_error(error_handler);
    bus.set_acknowledge(true);
    bus.set_crc_32(true);
    bus.set_packet_id(true);

    if (receiver) {
      std::thread run_server_thd(run_server, bind_ip);
      run_server_thd.detach();
    }

    if (transmitter) {
      std::thread grpc_client_thd(grpc_client, grpc_server_ip);
      grpc_client_thd.detach();
    }

    listen_on_bus();
    return 0;
  }
  catch (const char* msg) {
    std::cout << "exc: "
              << msg
              << std::endl;
    return 1;
  }
}
