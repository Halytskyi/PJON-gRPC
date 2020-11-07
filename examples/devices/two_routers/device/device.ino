/*
 *  This example show how possible control Arduino PINs or send messages to RPi/PC via 'router'
 *
 *  Examples (use python client 'pjon_grpc_client.py' on RPi/PC side):
 *  Read Hardware Digital PIN (digitalRead(13)): ./pjon_grpc_client.py 1 21 H-13
 *  Write Hardware Digital PIN (digitalWrite(13, 1)): ./pjon_grpc_client.py 1 21 H-13=1
 *  Increase number with each request: ./pjon_grpc_client.py 1 21 N
 *  Enable sending messages every 1 second from Arduino to RPi/PC: ./pjon_grpc_client.py 1 21 M=1
 *  Enable sending messages every 0.2 second from Arduino to RPi/PC: ./pjon_grpc_client.py 1 21 F=1
 *
 */

#define PJON_INCLUDE_PACKET_ID
#include <PJONSoftwareBitBang.h>

const byte deviceID = 21;
PJONSoftwareBitBang busA(deviceID); // TxRx bus
PJONSoftwareBitBang busB(deviceID); // Tx bus
const byte pinBusA = 7;
const byte pinBusB = 12;
const byte masterIdBusB = 6;
const int receiveTimeBusA = 7000;
const byte maxRequestLength = 15;

byte send_msg_repeatedly = 0;
byte send_msg_flood = 0;
unsigned long prevMillis_m = millis();
unsigned long prevMillis_mf = millis();
byte n = 0;
byte m = 0;
byte mf = 0;
const bool debug = false;


void busA_reply(const char request[], const char response[]) {
  char responseFull[maxRequestLength + 5];
  strcpy(responseFull, request);
  strcat(responseFull, ">");
  strcat(responseFull, response);
  busA.reply_blocking(responseFull, strlen(responseFull));
}

void busB_send(const char command[], const char response[]) {
  char responseFull[22]; // 21 charters + 1 NULL termination
  strcpy(responseFull, command);
  strcat(responseFull, "<");
  strcat(responseFull, response);
  busB.send_packet_blocking(masterIdBusB, responseFull, strlen(responseFull));
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
  char tmpBuf[8]; // for able storing number like "-127.00" + 1 NULL termination
  const char goodCommandReply[] = "ok";
  const char badCommandReply[] = "err";
  // Check if request not too long
  if (length > maxRequestLength) {
    char response[14] = "max_req_is_"; // Length string + 2 digits + 1 NULL termination
    itoa(maxRequestLength, tmpBuf, 10);
    strcat(response, tmpBuf);
    busA_reply(badCommandReply, response);
    return;
  }

  // Copy full request to new char array
  char request[length + 1];
  for (byte i = 0; i != length; i++) {
    request[i] = (char)payload[i];
  }
  request[length] = 0;

  const char delimiter[2] = "=";
  byte delimiterAddr = 0;
  byte commandLength = length;
  byte valueLength = 0;

  for (byte i = 0; i != length; i++) {
    if ((char)payload[i] == delimiter[0]) {
      delimiterAddr = i;
      commandLength = i;
      valueLength = length - i - 1;
      break;
    }
  }
  
  char command[commandLength + 1];
  char value[valueLength + 1];
  if (delimiterAddr == 0) {
    for (byte i = 0; i != length; i++) {
      command[i] = (char)payload[i];
    }
    command[commandLength] = 0;
    value[0] = 0;
  } else {
    for (byte i = 0; i != delimiterAddr; i++) {
      command[i] = (char)payload[i];
    }
    command[commandLength] = 0;
    for (byte i = 0; i != valueLength; i++) {
      value[i] = (char)payload[delimiterAddr + 1 + i];
    }
    value[valueLength] = 0;
  }

  // Return "err" if defined "delimiter" but no "value"
  if (delimiterAddr != 0 and valueLength == 0) {
    busA_reply(request, badCommandReply);
    return;
  }

  // Check if "value" contain no more 3 symbols and if it's consists only digits
  if (valueLength > 3) {
    busA_reply(request, badCommandReply);
    return;
  }
  for (byte i = 0; i != valueLength; i++) {
    if (! isDigit(value[i])) {
      busA_reply(request, badCommandReply);
      return;
    }
  }
  const byte value_int = atoi(value);

  // Requests
  if (strcmp(command, "H-13") == 0) {
    if (valueLength == 0) {
      itoa(digitalRead(13), tmpBuf, 10);
      busA_reply(request, tmpBuf);
      return;
    } else {
      if (value_int == 0 or value_int == 1) {
        digitalWrite(13, value_int);
        busA_reply(request, goodCommandReply);
        return;
      } else {
        busA_reply(request, badCommandReply);
        return;
      }
    }
  } else if (strcmp(command, "N") == 0) {
    n += 1;
    itoa(n, tmpBuf, 10);
    busA_reply(request, tmpBuf);
    return;
  } else if (strcmp(command, "M") == 0) {
    if (valueLength == 0) {
      itoa(send_msg_repeatedly, tmpBuf, 10);
      busA_reply(request, tmpBuf);
      return;
    } else {
      if (value_int == 0 or value_int == 1) {
        send_msg_repeatedly = value_int;
        m = 0;
        busA_reply(request, goodCommandReply);
        return;
      } else {
        busA_reply(request, badCommandReply);
        return;
      }
    }
  } else if (strcmp(command, "F") == 0) {
    if (valueLength == 0) {
      itoa(send_msg_flood, tmpBuf, 10);
      busA_reply(request, tmpBuf);
      return;
    } else {
      if (value_int == 0 or value_int == 1) {
        send_msg_flood = value_int;
        mf = 0;
        busA_reply(request, goodCommandReply);
        return;
      } else {
        busA_reply(request, badCommandReply);
        return;
      }
    }
  }
  busA_reply(request, badCommandReply);
}


void loop() {
  busA.receive(receiveTimeBusA);

  unsigned long curMillis = millis(); // time now in ms
  if (send_msg_repeatedly == 1 and curMillis - prevMillis_m >= 1000) {
    m += 1;
    char tmpBuf[] = "1 sec msg: ";
    char tmpBuf2[6];
    itoa(m, tmpBuf2, 10);
    strcat(tmpBuf, tmpBuf2);
    busB_send("M", tmpBuf);
    prevMillis_m = millis();
  }
  if (send_msg_flood == 1 and curMillis - prevMillis_mf >= 200) {
    mf += 1;
    char tmpBuf[] = "0.2 sec msg: ";
    char tmpBuf2[6];
    itoa(mf, tmpBuf2, 10);
    strcat(tmpBuf, tmpBuf2);
    busB_send("F", tmpBuf);
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

  busA.strategy.set_pin(pinBusA);
  busA.set_receiver(receiver_function);
  busA.set_acknowledge(true);
  busA.set_crc_32(true);
  busA.set_packet_id(true);
  busA.begin();

  busB.strategy.set_pin(pinBusB);
  busB.set_acknowledge(true);
  busB.set_crc_32(true);
  busB.set_packet_id(true);
  busB.begin();
};
