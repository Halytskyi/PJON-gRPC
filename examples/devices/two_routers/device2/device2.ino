/*
 *  This example show how possible control Arduino PINs or send messages to RPi/PC via 'router'
 *
 *  Examples (use python client 'pjon_grpc_client.py' on RPi/PC side):
 *  Read Hardware Digital PIN (digitalRead(13)): ./pjon_grpc_client.py 1 22 H-13
 *  Read Hardware Analog PIN (analogRead(14)): ./pjon_grpc_client.py 1 22 H-14
 *  Write Hardware Digital PIN (digitalWrite(13, 1)): ./pjon_grpc_client.py 1 22 H-13-1
 *  Read Virtual PIN: ./pjon_grpc_client.py 1 22 V-0
 *  Write Virtual PIN: ./pjon_grpc_client.py 1 22 V-0-1
 *  Increase number with each request: ./pjon_grpc_client.py 1 22 N
 *  Enable sending messages every 1 second from Arduino to RPi/PC: ./pjon_grpc_client.py 1 22 M-1
 *  Enable sending messages every 0.2 second from Arduino to RPi/PC: ./pjon_grpc_client.py 1 22 F-1
 *
 */

#define PJON_INCLUDE_PACKET_ID true
#define PJON_INCLUDE_SWBB true // Include SoftwareBitBang only
#include <PJON.h>

const uint8_t DEVICE_ID = 22;

PJON<SoftwareBitBang> busA(DEVICE_ID); // RxTx bus
PJON<SoftwareBitBang> busB(DEVICE_ID); // Rx bus

// For test virtual PINs
uint8_t vPIN[2] = { 0, 0 };

int master_id_busB = 2; // Master ID (appication 'pjon_grpc_server' running on RPi)
int send_msg_repeatedly = 0;
int send_msg_flood = 0;
unsigned long prevMillis_m = millis();
unsigned long prevMillis_mf = millis();
int n = 0;
int m = 0;
int mf = 0;
bool debug = false;

void bus_reply(String response_str) {
  int response_str_len = response_str.length();
  char response[response_str_len + 1];
  response_str.toCharArray(response, response_str_len + 1);
  busA.reply(response, response_str_len);
  busA.update();
}

void wrong_command(uint8_t *payload, uint16_t length) {
  String message_in = "";
  for (uint8_t i = 0; i != length; i++){
    message_in += (char)payload[i];
  }
  String response_str = message_in + ">wrong command";
  bus_reply(response_str);
  busA.update();
}

void error_handler_busA(uint8_t code, uint16_t data, void *custom_pointer) {
  if(code == PJON_CONNECTION_LOST) {
    Serial.print("Connection with device ID ");
    Serial.print(busA.packets[data].content[0], DEC);
    Serial.println(" is lost.");
  }
  if(code == PJON_PACKETS_BUFFER_FULL) {
    Serial.print("Packet buffer is full, has now a length of ");
    Serial.println(data);
    Serial.println("Possible wrong busA configuration!");
    Serial.println("higher PJON_MAX_PACKETS if necessary.");
  }
  if(code == PJON_CONTENT_TOO_LONG) {
    Serial.print("Content is too long, length: ");
    Serial.println(data);
  }
}

void error_handler_busB(uint8_t code, uint16_t data, void *custom_pointer) {
  if(code == PJON_CONNECTION_LOST) {
    Serial.print("Connection with device ID ");
    Serial.print(busB.packets[data].content[0], DEC);
    Serial.println(" is lost.");
  }
  if(code == PJON_PACKETS_BUFFER_FULL) {
    Serial.print("Packet buffer is full, has now a length of ");
    Serial.println(data);
    Serial.println("Possible wrong busB configuration!");
    Serial.println("higher PJON_MAX_PACKETS if necessary.");
  }
  if(code == PJON_CONTENT_TOO_LONG) {
    Serial.print("Content is too long, length: ");
    Serial.println(data);
  }
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
        m = 0;
        String response_str = "M-1>1";
        bus_reply(response_str);
      } else {
        wrong_command(payload, length);
      }
    } else {
      String response_str = "M>" + String(send_msg_repeatedly);
      bus_reply(response_str);
    }
  } else if ((char)payload[0] == 'F') {
    if ((char)payload[1] == '-') {
      if ((char)payload[2] == '0') {
        send_msg_flood = 0;
        String response_str = "F-0>0";
        bus_reply(response_str);
      } else if ((char)payload[2] == '1') {
        send_msg_flood = 1;
        mf = 0;
        String response_str = "F-1>1";
        bus_reply(response_str);
      } else {
        wrong_command(payload, length);
      }
    } else {
      String response_str = "F>" + String(send_msg_flood);
      bus_reply(response_str);
    }
  } else if ((char)payload[0] == 'N') {
    n += 1;
    String response_str = "N>" + String(n);
    bus_reply(response_str);
  } else {
    wrong_command(payload, length);
  }
};

void loop() {
  busA.receive(50000);
  busA.update();
  unsigned long curMillis = millis(); // time now in ms
  if (send_msg_repeatedly == 1 and curMillis - prevMillis_m >= 1000) {
    m += 1;
    String response_str_m = "<Message every 1 sec: " + String(m);
    int response_str_m_len = response_str_m.length();
    char response_m[response_str_m_len + 1];
    response_str_m.toCharArray(response_m, response_str_m_len + 1);
    busB.send_packet_blocking(master_id_busB, response_m, response_str_m_len);
    prevMillis_m = millis();
  }
  if (send_msg_flood == 1 and curMillis - prevMillis_mf >= 200) {
    mf += 1;
    String response_str_mf = "<Message every 0.2 sec: " + String(mf);
    int response_str_mf_len = response_str_mf.length();
    char response_mf[response_str_mf_len + 1];
    response_str_mf.toCharArray(response_mf, response_str_mf_len + 1);
    busB.send_packet_blocking(master_id_busB, response_mf, response_str_mf_len);
    prevMillis_mf = millis();
  }
};

void setup() {
  // For debug
  if (debug) {
    Serial.begin(115200);
    busA.set_error(error_handler_busA);
    busB.set_error(error_handler_busB);
  }

  // Initialize LED 13 to be off (for tests)
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  busA.strategy.set_pin(7);
  busA.set_receiver(receiver_function);
  busA.set_synchronous_acknowledge(true);
  busA.set_crc_32(true);
  busA.set_packet_id(true);
  busA.begin();

  busB.strategy.set_pin(12);
  busB.set_receiver(receiver_function);
  busB.set_synchronous_acknowledge(true);
  busB.set_crc_32(true);
  busB.set_packet_id(true);
  busB.begin();
};
