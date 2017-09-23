#define TS_RESPONSE_TIME_OUT 25000
#define PJON_INCLUDE_TS true // Include only ThroughSerial
#include <PJON.h>

PJON<ThroughSerial> bus(44);
int packet;

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  if ((char)payload[0] == 'R') {
    String data_reply = String(analogRead(17));
    int data_len = data_reply.length();
    char val[data_len + 1];
    data_reply.toCharArray(val, data_len + 1);
    if (!bus.packets[packet].state)
      packet = bus.reply(val, data_len);
  }
};

void setup() {
  Serial.begin(115200);
  bus.set_receiver(receiver_function);
  bus.strategy.set_serial(&Serial);
  bus.begin();
  bus.set_synchronous_acknowledge(true);
};

void loop() {
  bus.receive(1000);
  bus.update();
};
