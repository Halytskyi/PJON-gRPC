#define PJON_INCLUDE_SWBB true // Include SoftwareBitBang
#include <PJON.h>

PJON<SoftwareBitBang> bus(11);
int packet;
int test1 = 0;
int test2_enabled = 0;
int test2 = 0;
unsigned long prevMillis = millis();


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
  } else if ((char)payload[0] == 'T') {
    if ((char)payload[1] == ':') {
      String test_num = String((char)payload[2]);
      if (test_num == "1") {
        test1 += 1;
        char test1_char[16];
        itoa (test1, test1_char, 10);
        bus.reply(test1_char, String(test1).length());
      } else if (test_num == "2") {
        if ((char)payload[3] == ':') {
          String command = String((char)payload[4]);
          if (command == "0") {
            test2_enabled = 0;
            if (!bus.packets[packet].state)
              bus.reply("0", 1);
          } else if (command == "1") {
            test2_enabled = 1;
            if (!bus.packets[packet].state)
              bus.reply("1", 1);
          } else {
            if (!bus.packets[packet].state)
              bus.reply("non-existent command", 20);
          }
        } else {
          if (!bus.packets[packet].state) {
            char test2_enabled_char[16];
            itoa (test2_enabled, test2_enabled_char, 10);
            bus.reply(test2_enabled_char, 1);
          }
        }
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

void loop() {
  bus.receive(3000);
  bus.update();

  if (test2_enabled == 1) {
    unsigned long curMillis = millis(); // time now in ms
    if (curMillis - prevMillis >= 5000) {
      test2 += 1;
      char test2_char[16];
      itoa (test2, test2_char, 10);
      bus.send(1, test2_char, String(test2).length());
      prevMillis = curMillis;
    }
  }
};

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW); // Initialize LED 13 to be off

  bus.strategy.set_pin(12);
  bus.set_receiver(receiver_function);
  bus.set_synchronous_acknowledge(false);
  bus.set_crc_32(true);
  bus.begin();
};
