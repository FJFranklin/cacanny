#if defined(ARDUINO_MINIMA)

#include <Arduino_CAN.h>

#include "R4Canbus.hh"

static R4Canbus s_bus;

R4Canbus* R4Canbus::bus() {
  return &s_bus;
}

R4Canbus::R4Canbus() {
  // ...
}

R4Canbus::~R4Canbus() {
  // ...
}

bool R4Canbus::begin(LbBitrate bitrate) {
  CanBitRate CAN_BAUDRATE = CanBitRate::BR_250k; // see ArduinoCore-API/api/HardwareCAN.h

  if (bitrate != lb_250kbit) {
    if (bitrate == lb_1Mbit)
      CAN_BAUDRATE = CanBitRate::BR_1000k;
    else
      return false;
  }
  if (!CAN.begin(CAN_BAUDRATE)) {
    return false;
  }

  return true;
}

void R4Canbus::send(uint32_t packet_id, uint8_t* bytes, int count) {
  CAN.write(CanMsg(packet_id, count, bytes));
}

void R4Canbus::spin() {
  if (!CAN.available()) return;

  CanMsg const msg = CAN.read();

  uint32_t packet_id = msg.getExtendedId();

  // RTR reserved but not implemented

  // bool bExtended = msg.isExtendedId(); // uses 29-bit id instead of 11-bit id

  data_received(packet_id, msg.data, msg.data_length);
}

#endif // ARDUINO_MINIMA
