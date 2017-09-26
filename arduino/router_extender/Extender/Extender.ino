#define TS_RESPONSE_TIME_OUT 35000
#define PJON_INCLUDE_TS true // Include ThroughSerial
#define PJON_INCLUDE_SWBB true // Include SoftwareBitBang
#include <PJON.h>

const uint8_t DEVICE_ID = 50;

PJON<ThroughSerial> busA(DEVICE_ID);
PJON<SoftwareBitBang> busB(DEVICE_ID);

// All packets to devices listed here will be forwarded to bus B
const uint8_t device_id_ranges_on_B_side[] = {11, 20};


void toggle_led() {
  static bool led_is_on = false;
  led_is_on = !led_is_on;
  digitalWrite(13, led_is_on ? HIGH : LOW);
}

bool is_device_on_B_side(uint8_t device_id) {
  // Check if in one of the B ranges
  for (uint8_t i = 0; i < sizeof device_id_ranges_on_B_side - 1; i += 2)
    if (device_id_ranges_on_B_side[i] <= device_id && device_id <= device_id_ranges_on_B_side[i+1])
      return true;
  return false;
}

void receiver_functionA(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Forward packet to B segment of local bus
  if (is_device_on_B_side(packet_info.receiver_id)) {
    busA.strategy.send_response(PJON_ACK);
    toggle_led();
    busB.send_from_id(packet_info.sender_id, packet_info.sender_bus_id,
      packet_info.receiver_id, packet_info.receiver_bus_id, payload, length, packet_info.header);
  }
}

void receiver_functionB(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // Forward packet to A segment of local bus
  if (!is_device_on_B_side(packet_info.receiver_id)) {
    busB.strategy.send_response(PJON_ACK);
    toggle_led();
    busA.send_from_id(packet_info.sender_id, packet_info.sender_bus_id,
      packet_info.receiver_id, packet_info.receiver_bus_id, payload, length, packet_info.header);
  }
}

void loop() {
  busA.receive(1000);
  busB.update();
  busB.receive(1000);
  busA.update();
};

void setup() {
  Serial.begin(115200);
  // Initialize LED 13 to be off (for tests)
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  busA.strategy.set_serial(&Serial);
  busA.set_receiver(receiver_functionA);
  busA.set_synchronous_acknowledge(true);
  busA.set_crc_32(true);
  busA.set_router(true);
  busA.begin();

  busB.strategy.set_pin(7);
  busB.set_receiver(receiver_functionB);
  busB.set_crc_32(true);
  busB.set_router(true);
  busB.begin();
}
