/*
 *  This sketch is example how can control PINs via PJON-gRPC tool
 *
 *  Examples (with use python client on RPi side):
 *  Read Hardware Digital PIN (digitalRead(13)): ./pjon_grpc_client.py 44 H:13
 *  Read Hardware Analog PIN (analogRead(14)): ./pjon_grpc_client.py 44 H:14
 *  Write Hardware Digital PIN (digitalWrite(13, 1)): ./pjon_grpc_client.py 44 H:13:1
 *  Read Virtual PIN: ./pjon_grpc_client.py 44 V:0
 *  Write Virtual PIN: ./pjon_grpc_client.py 44 V:0:1
 *
 */

#define TS_RESPONSE_TIME_OUT 35000
#define PJON_INCLUDE_TS true // Include only ThroughSerial
#include <PJON.h>

PJON<ThroughSerial> bus(44);
int packet;

// For test virtual PINs
uint8_t vPIN[2] = { 0, 0 };

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Hardware PINs
  if ((char)payload[0] == 'H') {
    if ((char)payload[1] == ':') {
      String pin_num = String((char)payload[2]);
      if ((char)payload[3] != ':')
        pin_num += String((char)payload[3]);
      int pin_num_int = pin_num.toInt();
      if (pin_num_int > 1 && pin_num_int < 14) {
        if ((char)payload[3] == ':' || (char)payload[4] == ':') {
          int pin_status;
          if ((char)payload[3] == ':') {
            pin_status = (char)payload[4] - '0';
          } else {
            pin_status = (char)payload[5] - '0';
          }
          if (pin_status == 0 || pin_status == 1) {
            digitalWrite(pin_num_int, pin_status);
          } else {
            if (!bus.packets[packet].state)
              packet = bus.reply("non-existent command", 20);
          }
        }
        String data_reply = String(digitalRead(pin_num_int));
        int data_len = data_reply.length();
        char data_reply_char[data_len + 1];
        data_reply.toCharArray(data_reply_char, data_len + 1);
        if (!bus.packets[packet].state)
          packet = bus.reply(data_reply_char, data_len);
      } else if (pin_num_int > 13 && pin_num_int < 22) {
        String data_reply = String(analogRead(pin_num_int));
        int data_len = data_reply.length();
        char data_reply_char[data_len + 1];
        data_reply.toCharArray(data_reply_char, data_len + 1);
        if (!bus.packets[packet].state)
          packet = bus.reply(data_reply_char, data_len);
      } else {
        if (!bus.packets[packet].state)
          packet = bus.reply("non-existent command", 20);
      }
    } else {
      if (!bus.packets[packet].state)
        packet = bus.reply("non-existent command", 20);
    }
  // Virtual PINs
  } else if ((char)payload[0] == 'V') {
    if ((char)payload[1] == ':') {
      String pin_num = String((char)payload[2]);
      if ((char)payload[3] != ':')
        pin_num += String((char)payload[3]);
      int pin_num_int = pin_num.toInt();
      if (pin_num_int >= 0 && pin_num_int < 2) {
        if ((char)payload[3] == ':' || (char)payload[4] == ':') {
          int pin_status;
          if ((char)payload[3] == ':') {
            pin_status = (char)payload[4] - '0';
          } else {
            pin_status = (char)payload[5] - '0';
          }
          if (pin_status == 0 || pin_status == 1) {
            vPIN[pin_num_int] = pin_status;
          } else {
            if (!bus.packets[packet].state)
              packet = bus.reply("non-existent command", 20);
          }
        }
        String data_reply = String(vPIN[pin_num_int]);
        int data_len = data_reply.length();
        char data_reply_char[data_len + 1];
        data_reply.toCharArray(data_reply_char, data_len + 1);
        if (!bus.packets[packet].state)
          packet = bus.reply(data_reply_char, data_len);
      } else {
        if (!bus.packets[packet].state)
          packet = bus.reply("non-existent command", 20);
      }
    } else {
      if (!bus.packets[packet].state)
        packet = bus.reply("non-existent command", 20);
    }
  } else {
    if (!bus.packets[packet].state)
      packet = bus.reply("non-existent command", 20);
  }
};

void setup() {
  Serial.begin(115200);

  // Initialize LED 13 to be off (for tests)
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  bus.set_receiver(receiver_function);
  bus.strategy.set_serial(&Serial);
  bus.begin();
  bus.set_synchronous_acknowledge(true);
  // crc_8 doesn't work correctly with 8.x PJON version, can be fixed in v9.x
  bus.set_crc_32(true);
};

void loop() {
  bus.receive(1000);
  bus.update();
};
