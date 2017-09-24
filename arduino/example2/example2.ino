#define TS_RESPONSE_TIME_OUT 35000
#define PJON_INCLUDE_TS true // Include only ThroughSerial
#include <PJON.h>

PJON<ThroughSerial> bus(44);
int packet;

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  if ((char)payload[0] == 'H') {
    if ((char)payload[1] == ':') {
      String pin_num = String((char)payload[2]);
      if ((char)payload[3] != ':')
        pin_num += String((char)payload[3]);
      int pin_num_int = pin_num.toInt();
//      if (pin_num_int > 1 && pin_num_int < 14) {
      if (pin_num_int) {
        if ((char)payload[3] == ':' || (char)payload[4] == ':') {
          int pin_status;
          if ((char)payload[3] == ':') {
            pin_status = (char)payload[4] - '0';
          } else {
            pin_status = (char)payload[5] - '0';
          }
          if (pin_status == 0 || pin_status == 1) {
            digitalWrite(pin_num_int, pin_status);
          }
        }
//        String data_reply = String(digitalRead(pin_num_int));
        String data_reply = String(pin_num_int);
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
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW); // Initialize LED 13 to be off

  bus.set_receiver(receiver_function);
  bus.strategy.set_serial(&Serial);
  bus.begin();
  bus.set_synchronous_acknowledge(true);
};

void loop() {
  bus.receive(1000);
  bus.update();
};
