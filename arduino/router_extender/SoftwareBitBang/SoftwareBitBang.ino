#define PJON_INCLUDE_SWBB true // Include SoftwareBitBang
#include <PJON.h>

PJON<SoftwareBitBang> bus(11);


void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  /* Make use of the payload before sending something, the buffer where payload points to is
     overwritten when a new message is dispatched */
  if(payload[0] == 'B') {
//    digitalWrite(13, HIGH);
//    delay(30);
//    digitalWrite(13, LOW);
    bus.reply("B", 1);
  }
}

void loop() {
  bus.receive(1000);
  bus.update();
};

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW); // Initialize LED 13 to be off

  bus.strategy.set_pin(7);
  bus.set_receiver(receiver_function);
  bus.set_crc_32(true);
  bus.begin();
};
