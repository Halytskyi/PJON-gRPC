#define PJON_MAX_PACKETS 4 // reduced for decrease memory usage
#define PJON_INCLUDE_PACKET_ID true
#define TSA_RESPONSE_TIME_OUT 100000
#define PJON_INCLUDE_TSA true
#define PJON_INCLUDE_SWBB true
#include <PJON.h>
#include <SoftwareSerial.h>

const uint8_t DEVICE_ID = 11;

SoftwareSerial HC12(2, 3);
PJON<ThroughSerialAsync> busA(DEVICE_ID);
PJON<SoftwareBitBang> busB(DEVICE_ID);
PJON<ThroughSerialAsync> busC(DEVICE_ID);

const uint8_t device_id_ranges_on_B_side[] = {21, 50};
const uint8_t device_id_ranges_on_C_side[] = {51, 80};


bool is_device_on_B_side(uint8_t device_id) {
  // Check if in one of the B ranges
  for (uint8_t i = 0; i < sizeof device_id_ranges_on_B_side - 1; i += 2)
    if (device_id_ranges_on_B_side[i] <= device_id && device_id <= device_id_ranges_on_B_side[i+1])
      return true;
  return false;
}

bool is_device_on_C_side(uint8_t device_id) {
  // Check if in one of the C ranges
  for (uint8_t i = 0; i < sizeof device_id_ranges_on_C_side - 1; i += 2)
    if (device_id_ranges_on_C_side[i] <= device_id && device_id <= device_id_ranges_on_C_side[i+1])
      return true;
  return false;
}

void receiver_functionA(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Forward packet to B/C segment of local bus
  if (is_device_on_B_side(packet_info.receiver_id)) {
    busA.strategy.send_response(PJON_ACK);
    busB.send_from_id(
      packet_info.sender_id,
      packet_info.sender_bus_id,
      packet_info.receiver_id,
      packet_info.receiver_bus_id,
      (char *)payload,
      length,
      packet_info.header,
      packet_info.id,
      packet_info.port
    );
  } else if (is_device_on_C_side(packet_info.receiver_id)) {
    busA.strategy.send_response(PJON_ACK);
    busC.send_from_id(
      packet_info.sender_id,
      packet_info.sender_bus_id,
      packet_info.receiver_id,
      packet_info.receiver_bus_id,
      (char *)payload,
      length,
      packet_info.header,
      packet_info.id,
      packet_info.port
    );
  }
}

void receiver_functionB(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Forward packet to A segment of local bus
  if (!is_device_on_B_side(packet_info.receiver_id)) {
    busB.strategy.send_response(PJON_ACK);
    busA.send_from_id(
      packet_info.sender_id,
      packet_info.sender_bus_id,
      packet_info.receiver_id,
      packet_info.receiver_bus_id,
      (char *)payload,
      length,
      packet_info.header,
      packet_info.id,
      packet_info.port
    );
  }
}

void receiver_functionC(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Forward packet to A segment of local bus
  if (!is_device_on_C_side(packet_info.receiver_id)) {
    busC.strategy.send_response(PJON_ACK);
    busA.send_from_id(
      packet_info.sender_id,
      packet_info.sender_bus_id,
      packet_info.receiver_id,
      packet_info.receiver_bus_id,
      (char *)payload,
      length,
      packet_info.header,
      packet_info.id,
      packet_info.port
    );
  }
}

void loop() {
  busA.receive();
  busA.update();
  busB.receive();
  busA.update();
  busC.receive();
  busA.update();
  busB.update();
  busC.update();
};

void setup() {
  Serial.begin(9600);

  // RPi
  busA.strategy.set_serial(&Serial);
  busA.set_receiver(receiver_functionA);
  busA.set_synchronous_acknowledge(true);
  busA.set_crc_32(true);
  busA.set_router(true);
  busA.set_packet_id(true);
  busA.begin();

  // Devices via SoftwareBitBang
  busB.strategy.set_pin(12);
  busB.set_receiver(receiver_functionB);
  busB.set_synchronous_acknowledge(true);
  busB.set_crc_32(true);
  busB.set_router(true);
  busB.set_packet_id(true);
  busB.begin();

  // Devices via ThroughSerialAsync (HC-12 module)
  HC12.begin(9600);
  busC.strategy.set_serial(&HC12);
  busC.set_receiver(receiver_functionC);
  busC.set_synchronous_acknowledge(true);
  busC.set_crc_32(true);
  busC.set_router(true);
  busC.set_packet_id(true);
  busC.begin();
}
