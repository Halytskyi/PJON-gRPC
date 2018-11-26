/*
 *  This example show how possible control Arduino PINs or send messages to RPi/PC via PJON-gRPC tool
 *
 *  Examples (use python client 'pjon_grpc_client.py' on RPi/PC side):
 *  Read Hardware Digital PIN (digitalRead(13)): ./pjon_grpc_client.py 1 11 H-13
 *  Read Hardware Analog PIN (analogRead(14)): ./pjon_grpc_client.py 1 11 H-14
 *  Write Hardware Digital PIN (digitalWrite(13, 1)): ./pjon_grpc_client.py 1 11 H-13-1
 *  Read Virtual PIN: ./pjon_grpc_client.py 1 11 V-0
 *  Write Virtual PIN: ./pjon_grpc_client.py 1 11 V-0-1
 *  Enable sending messages every 1 second from Arduino to RPi/PC: ./pjon_grpc_client.py 1 11 M-1
 *
 */

#define PJON_INCLUDE_PACKET_ID true
#define TSA_RESPONSE_TIME_OUT 100000
#define PJON_INCLUDE_TSA true // Include only ThroughSerial
#include <PJON.h>

PJON<ThroughSerialAsync> bus(11);

// For test virtual PINs
uint8_t vPIN[2] = { 0, 0 };

int master_id = 1; // Master ID (appication 'pjon_grpc_server' running on RPi)
int send_msg_repeatedly = 0;
unsigned long prevMillis = millis();

void bus_reply(String response_str) {
  int response_str_len = response_str.length();
  char response[response_str_len + 1];
  response_str.toCharArray(response, response_str_len + 1);
  bus.reply(response, response_str_len);
  bus.update();
}

void wrong_command(uint8_t *payload, uint16_t length) {
  String message_in = "";
  for (uint8_t i = 0; i != length; i++){
    message_in += (char)payload[i];
  }
  String response_str = message_in + ">wrong command";
  bus_reply(response_str);
  bus.update();
}

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Hardware PINs
  if ((char)payload[0] == 'H') {
    if ((char)payload[1] == '-') {
      String pin_num = String((char)payload[2]);
      if ((char)payload[3] != '-')
        pin_num += String((char)payload[3]);
      int pin_num_int = pin_num.toInt();
      if (pin_num_int > 1 && pin_num_int < 14) {
        int pin_status;
        if ((char)payload[3] == '-' || (char)payload[4] == '-') {
          if ((char)payload[3] == '-') {
            pin_status = (char)payload[4] - '0';
          } else {
            pin_status = (char)payload[5] - '0';
          }
          if (pin_status == 0 || pin_status == 1) {
            digitalWrite(pin_num_int, pin_status);
            String response_str = "H-" + pin_num + "-" + String(pin_status) + ">" + String(digitalRead(pin_num_int));
            bus_reply(response_str);
          } else {
            wrong_command(payload, length);
          }
        } else {
          String response_str = "H-" + pin_num + ">" + String(digitalRead(pin_num_int));
          bus_reply(response_str);
        }
      } else if (pin_num_int > 13 && pin_num_int < 22) {
        String response_str = "H-" + pin_num + ">" + String(analogRead(pin_num_int));
        bus_reply(response_str);
      } else {
        wrong_command(payload, length);
      }
    } else {
      wrong_command(payload, length);
    }
  // Virtual PINs
  } else if ((char)payload[0] == 'V') {
    if ((char)payload[1] == '-') {
      int pin_num_int = String((char)payload[2]).toInt();
      if (pin_num_int >= 0 && pin_num_int < 2) {
        if ((char)payload[3] == '-') {
          int pin_status = (char)payload[4] - '0';
          if (pin_status == 0 || pin_status == 1) {
            vPIN[pin_num_int] = pin_status;
            String response_str = "V-" + String(pin_num_int) + "-" + String(pin_status) + ">" + String(vPIN[pin_num_int]);
            bus_reply(response_str);
          } else {
            wrong_command(payload, length);
          }
        } else {
          String response_str = "V-" + String(pin_num_int) + ">" + String(vPIN[pin_num_int]);
          bus_reply(response_str);
        }
      } else {
        wrong_command(payload, length);
      }
    } else {
      wrong_command(payload, length);
    }
  } else if ((char)payload[0] == 'M') {
    if ((char)payload[1] == '-') {
      if ((char)payload[2] == '0') {
        send_msg_repeatedly = 0;
        String response_str = "M-0>0";
        bus_reply(response_str);
      } else if ((char)payload[2] == '1') {
        send_msg_repeatedly = 1;
        String response_str = "M-1>1";
        bus_reply(response_str);
      } else {
        wrong_command(payload, length);
      }
    } else {
      String response_str = "M>" + String(send_msg_repeatedly);
      bus_reply(response_str);
    }
  } else {
    wrong_command(payload, length);
  }
};

void loop() {
  bus.receive();
  bus.update();
  unsigned long curMillis = millis(); // time now in ms
  if (send_msg_repeatedly == 1 and curMillis - prevMillis >= 1000) {
    bus.send(master_id, "<Incoming message every 1 sec", 29);
    bus.update();
    prevMillis = millis();
  }
};

void setup() {
  Serial.begin(9600);

  // Initialize LED 13 to be off (for tests)
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  bus.strategy.set_serial(&Serial);
  bus.set_receiver(receiver_function);
  bus.set_synchronous_acknowledge(true);
  bus.set_crc_32(true);
  bus.set_packet_id(true);
  bus.begin();
};
