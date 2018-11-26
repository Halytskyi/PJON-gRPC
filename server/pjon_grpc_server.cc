// Common libraries
#include "INIReader.h"
#include "version.h"
#include <stdio.h>
#include <thread>
#include <queue>
#include <vector>
#include <ctime>
// PJON library
#define PJON_INCLUDE_PACKET_ID true
#define TSA_RESPONSE_TIME_OUT 100000
#define PJON_INCLUDE_TSA true
#ifndef RPI
  #define RPI true
#endif
#include "PJON.h"
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

// Get values from configuration file
std::string config_file = "conf/pjon-grpc.cfg";
INIReader reader(config_file);
// Module 1
int mod1_master_id = reader.GetInteger("module-1", "master-id", 1);
bool mod1_debug = reader.GetBoolean("module-1", "debug", false);
bool mod1_receiver = reader.GetBoolean("module-1", "receiver", false);
int mod1_retry_requests = reader.GetInteger("module-1", "retry-requests", 0);
std::string mod1_delimiter_transmit = reader.Get("module-1", "delimiter-transmit", ">");
bool mod1_transmitter = reader.GetBoolean("module-1", "transmitter", false);
std::string mod1_delimiter_receive = reader.Get("module-1", "delimiter-receive", "<");
// Module 2
int mod2_master_id = reader.GetInteger("module-2", "master-id", 2);
bool mod2_debug = reader.GetBoolean("module-2", "debug", false);
bool mod2_receiver = reader.GetBoolean("module-2", "receiver", false);
int mod2_retry_requests = reader.GetInteger("module-2", "retry-requests", 0);
std::string mod2_delimiter_transmit = reader.Get("module-2", "delimiter-transmit", ">");
bool mod2_transmitter = reader.GetBoolean("module-2", "transmitter", false);
std::string mod2_delimiter_receive = reader.Get("module-2", "delimiter-receive", "<");

PJON<ThroughSerialAsync> bus1(mod1_master_id);
PJON<ThroughSerialAsync> bus2(mod2_master_id);

uint64_t rcvd_cnt = 0;
// Module 1
uint64_t mod1_rcvd_cnt = 0;
std::vector<std::array<std::string, 3>> mod1_check_messages_array;
std::vector<std::string> mod1_receives_array = {"", ""};
std::queue<std::vector<std::string>> mod1_receives_queue;
// Module 2
uint64_t mod2_rcvd_cnt = 0;
std::vector<std::array<std::string, 3>> mod2_check_messages_array;
std::vector<std::string> mod2_receives_array = {"", ""};
std::queue<std::vector<std::string>> mod2_receives_queue;


void message_processing(std::string response, int packet_sender_id, int packet_receiver_id) {
  if (packet_receiver_id == mod1_master_id) {
    if (mod1_receiver and response.find(mod1_delimiter_transmit) != std::string::npos) {
      mod1_check_messages_array.push_back({std::to_string(packet_sender_id), response, std::to_string(time(0))});
    } else if (mod1_transmitter and response.find(mod1_delimiter_receive) != std::string::npos) {
      mod1_receives_array[0] = std::to_string(packet_sender_id);
      mod1_receives_array[1] = response;
      mod1_receives_queue.push(mod1_receives_array);
    }
  } else if (packet_receiver_id == mod2_master_id) {
    if (mod2_receiver and response.find(mod2_delimiter_transmit) != std::string::npos) {
      mod2_check_messages_array.push_back({std::to_string(packet_sender_id), response, std::to_string(time(0))});
    } else if (mod2_transmitter and response.find(mod2_delimiter_receive) != std::string::npos) {
      mod2_receives_array[0] = std::to_string(packet_sender_id);
      mod2_receives_array[1] = response;
      mod2_receives_queue.push(mod2_receives_array);
    }
  }
}

static void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  std::string response = "";
  for (uint32_t i = 0; i != length; i++){
    response += payload[i];
  }
  if ((mod1_debug and packet_info.receiver_id == mod1_master_id) or (mod2_debug and packet_info.receiver_id == mod2_master_id)) {
    if (packet_info.receiver_id == mod1_master_id) {
      mod1_rcvd_cnt += 1;
      rcvd_cnt = mod1_rcvd_cnt;
    } else if (packet_info.receiver_id == mod2_master_id) {
      mod2_rcvd_cnt += 1;
      rcvd_cnt = mod2_rcvd_cnt;
    }
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
  message_processing(response, packet_info.sender_id, packet_info.receiver_id);
};

static void error_handler_function(uint8_t code, uint16_t data, void *custom_pointer) {
  if(code == PJON_CONNECTION_LOST) {
    std::string error_device;
    if (mod1_receiver or mod1_transmitter) {
      error_device = std::to_string(bus1.packets[data].content[0]);
    } else if (mod2_receiver or mod2_transmitter) {
      error_device = std::to_string(bus2.packets[data].content[0]);
    }
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

// Server for Module 1
void run_server1(std::string bind_ip) {
  class ArduinoServiceImpl final : public Arduino::Service {
    Status RPiArduino(ServerContext* context, const Arduino_Request* request,
                    Arduino_Reply* reply) override {
      int node_id = request->node_id();
      const char* data = request->data().c_str();
      std::string response;
      int send_requests = mod1_retry_requests + 1;
      while(send_requests != 0) {
        if (mod1_debug) {
          std::cout << "Received command for module-1: " << data << std::endl;
          std::cout << "Attempting to send a packet from module-1..." << std::endl;
        }
        bus1.send(node_id, data, strlen(data));
        if (mod1_debug)
          std::cout << "Attempting to roll bus for module-1..." << std::endl;
        bus1.update();
        uint32_t time_start = micros();
        while(micros() - time_start < 1000000) {
          if (mod1_check_messages_array.size() != 0) {
            for (int i=0; i<mod1_check_messages_array.size(); ++i) {
              std::string response_data = mod1_check_messages_array[i][1].substr(0, mod1_check_messages_array[i][1].find(mod1_delimiter_transmit));
              if (time(0) - 3 >= std::stoi(mod1_check_messages_array[i][2])) {
                mod1_check_messages_array.erase(mod1_check_messages_array.begin()+i);
                i -= 1;
              } else if (mod1_check_messages_array[i][0] == std::to_string(node_id) and response_data == data) {
                response = mod1_check_messages_array[i][1];
                mod1_check_messages_array.erase(mod1_check_messages_array.begin()+i);
                time_start = micros() - 1000000;
                send_requests = 1;
                break;
              }
            }
          }
        }
        send_requests -= 1;
      }
      if (mod1_debug) {
        std::cout << "Client response for module-1: " << response << std::endl;
        std::cout << "Array size for module-1: " << mod1_check_messages_array.size() << std::endl << std::endl;
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
  if (mod1_debug){
    std::cout << "Module-1: Server listening on " << server_address << std::endl;
  }
  server->Wait();
}

// Server for Module 2
void run_server2(std::string bind_ip) {
  class ArduinoServiceImpl final : public Arduino::Service {
    Status RPiArduino(ServerContext* context, const Arduino_Request* request,
                    Arduino_Reply* reply) override {
      int node_id = request->node_id();
      const char* data = request->data().c_str();
      std::string response;
      int send_requests = mod2_retry_requests + 1;
      while(send_requests != 0) {
        if (mod2_debug) {
          std::cout << "Received command for module-2: " << data << std::endl;
          std::cout << "Attempting to send a packet from module-2..." << std::endl;
        }
        bus2.send(node_id, data, strlen(data));
        if (mod2_debug)
          std::cout << "Attempting to roll bus for module-2..." << std::endl;
        bus2.update();
        uint32_t time_start = micros();
        while(micros() - time_start < 1000000) {
          if (mod2_check_messages_array.size() != 0) {
            for (int i=0; i<mod2_check_messages_array.size(); ++i) {
              std::string response_data = mod2_check_messages_array[i][1].substr(0, mod2_check_messages_array[i][1].find(mod2_delimiter_transmit));
              if (time(0) - 3 >= std::stoi(mod2_check_messages_array[i][2])) {
                mod2_check_messages_array.erase(mod2_check_messages_array.begin()+i);
                i -= 1;
              } else if (mod2_check_messages_array[i][0] == std::to_string(node_id) and response_data == data) {
                response = mod2_check_messages_array[i][1];
                mod2_check_messages_array.erase(mod2_check_messages_array.begin()+i);
                time_start = micros() - 1000000;
                send_requests = 1;
                break;
              }
            }
          }
        }
        send_requests -= 1;
      }
      if (mod2_debug) {
        std::cout << "Client response for module-2: " << response << std::endl;
        std::cout << "Array size for module-2: " << mod2_check_messages_array.size() << std::endl << std::endl;
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
  if (mod2_debug){
    std::cout << "Module-2: Server listening on " << server_address << std::endl;
  }
  server->Wait();
}

void grpc_client(std::string grpc_server_ip, int module_num) {
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
        if (mod1_debug or mod2_debug) {
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
        }
        return "RPC failed";
      }
    }
   private:
    std::unique_ptr<Arduino::Stub> stub_;
  };

  ArduinoClient arduino(grpc::CreateChannel(grpc_server_ip, grpc::InsecureChannelCredentials()));
  if (module_num == 1) {
    while(true) {
      if (mod1_receives_queue.size() != 0) {
        while(!mod1_receives_queue.empty()) {
          int node_id = stoi(mod1_receives_queue.front()[0]);
          std::string data = mod1_receives_queue.front()[1];
          std::string reply = arduino.RPiArduino(node_id, data);
          if (mod1_debug)
            std::cout << "Module-1: gRPC server answer: " << reply << std::endl;
          if (reply == "done")
            mod1_receives_queue.pop();
          delayMicroseconds(100000);
        }
      }
      delayMicroseconds(100000);
    }
  } else if (module_num == 2) {
    while(true) {
      if (mod2_receives_queue.size() != 0) {
        while(!mod2_receives_queue.empty()) {
          int node_id = stoi(mod2_receives_queue.front()[0]);
          std::string data = mod2_receives_queue.front()[1];
          std::string reply = arduino.RPiArduino(node_id, data);
          if (mod2_debug)
            std::cout << "Module-2: gRPC server answer: " << reply << std::endl;
          if (reply == "done")
            mod2_receives_queue.pop();
          delayMicroseconds(100000);
        }
      }
      delayMicroseconds(100000);
    }
  }
}

void listen_on_bus() {
  while(true) {
    if (mod1_receiver or mod1_transmitter) {
      bus1.update();
      bus1.receive();
    }
    if (mod2_receiver or mod2_transmitter) {
      bus2.update();
      bus2.receive();
    }
    delayMicroseconds(300);
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

  // Module 1
  const char* mod1_serial_device = reader.Get("module-1", "serial-device", "/dev/ttyUSB0").c_str();
  int mod1_bitrate = reader.GetInteger("module-1", "bitrate", 9600);
  std::string mod1_bind_ip = reader.Get("module-1", "bind-ip", "0.0.0.0:50051");
  std::string mod1_grpc_server_ip = reader.Get("module-1", "grpc-server-ip", "127.0.0.1:50061");

  if (std::string(mod1_serial_device).find("/dev/tty") == std::string::npos) {
    std::cout << "Wrong value for 'serial-device' parameter of module-1: '" << mod1_serial_device <<"', should be '/dev/ttyXXXX'\n";
    return 1;
  } else if (mod1_bitrate < 2400 or mod1_bitrate > 153600) {
    std::cout << "Wrong value for 'bitrate' parameter of module-1: '" << mod1_bitrate <<"', should specify bitrate '1 - 153600', like: 2400, 9600, 19200, 38400, 57600, 115200, 153600\n";
    return 1;
  }

  // Module 2
  const char* mod2_serial_device = reader.Get("module-2", "serial-device", "/dev/ttyUSB1").c_str();
  int mod2_bitrate = reader.GetInteger("module-2", "bitrate", 9600);
  std::string mod2_bind_ip = reader.Get("module-2", "bind-ip", "0.0.0.0:50052");
  std::string mod2_grpc_server_ip = reader.Get("module-2", "grpc-server-ip", "127.0.0.1:50062");

  if (std::string(mod2_serial_device).find("/dev/tty") == std::string::npos) {
    std::cout << "Wrong value for 'serial-device' parameter of module-2: '" << mod2_serial_device <<"', should be '/dev/ttyXXXX'\n";
    return 1;
  } else if (mod2_bitrate < 2400 or mod2_bitrate > 153600) {
    std::cout << "Wrong value for 'bitrate' parameter of module-2: '" << mod2_bitrate <<"', should specify bitrate '1 - 153600', like: 2400, 9600, 19200, 38400, 57600, 115200, 153600\n";
    return 1;
  }

  if (!mod1_transmitter and !mod1_receiver and !mod2_receiver and !mod2_transmitter) {
    std::cout << "'transmitter' and 'receiver' disabled for all modules. You should enable at least one of them" << std::endl;
    return 1;
  }

  try {
    if (wiringPiSetup() == -1) {
      std::cout << "WiringPi setup fail" << std::endl;
      exit (EXIT_FAILURE);
    }

    // Module 1
    if (mod1_receiver or mod1_transmitter) {
      if (mod1_debug)
        std::cout << "Opening serial for module 1..." << std::endl;
      int s1 = serialOpen(mod1_serial_device, mod1_bitrate);
      if (int(s1) < 0) {
        std::cout << "Serial open fail for module 1!" << std::endl;
        exit (EXIT_FAILURE);
      }
      if (mod1_debug)
        std::cout << "Setting serial for module 1..." << std::endl;
      bus1.strategy.set_serial(s1);
      bus1.strategy.set_baud_rate(mod1_bitrate);
      if (mod1_debug)
        std::cout << "Opening bus for module 1..." << std::endl;
      bus1.begin();
      bus1.set_receiver(receiver_function);
      if (mod1_debug)
        bus1.set_error(error_handler_function);
      bus1.set_synchronous_acknowledge(true);
      bus1.set_crc_32(true);
      bus1.set_packet_id(true);
    }

    // Module 2
    if (mod2_receiver or mod2_transmitter) {
      if (mod2_debug)
        std::cout << "Opening serial for module 2..." << std::endl;
      int s2 = serialOpen(mod2_serial_device, mod2_bitrate);
      if (int(s2) < 0) {
        std::cout << "Serial open fail for module 2!" << std::endl;
        exit (EXIT_FAILURE);
      }
      if (mod2_debug)
        std::cout << "Setting serial for module 2..." << std::endl;
      bus2.strategy.set_serial(s2);
      bus2.strategy.set_baud_rate(mod2_bitrate);
      if (mod2_debug)
        std::cout << "Opening bus for module 2..." << std::endl;
      bus2.begin();
      bus2.set_receiver(receiver_function);
      if (mod2_debug)
        bus2.set_error(error_handler_function);
      bus2.set_synchronous_acknowledge(true);
      bus2.set_crc_32(true);
      bus2.set_packet_id(true);
    }

    if (mod1_receiver) {
      std::thread run_server_thd1(run_server1, mod1_bind_ip);
      run_server_thd1.detach();
    }
    if (mod2_receiver) {
      delayMicroseconds(500000);
      std::thread run_server_thd2(run_server2, mod2_bind_ip);
      run_server_thd2.detach();
    }

    if (mod1_transmitter) {
      std::thread grpc_client_thd1(grpc_client, mod1_grpc_server_ip, 1);
      grpc_client_thd1.detach();
    }
    if (mod2_transmitter) {
      std::thread grpc_client_thd2(grpc_client, mod2_grpc_server_ip, 2);
      grpc_client_thd2.detach();
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
