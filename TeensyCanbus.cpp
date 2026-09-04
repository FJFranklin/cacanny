/* Copyright 2026 Francis James Franklin
 * MIT license: See LICENSE file.
 */

#if defined(CORE_TEENSY)

#include <FlexCAN_T4.h>

#include "TeensyCanbus.hh"

static FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> s_can;

static TeensyCanbus s_bus;

static void s_sniff(const CAN_message_t &msg) {
  s_bus.receive(msg);
}

TeensyCanbus* TeensyCanbus::bus() {
  return &s_bus;
}

TeensyCanbus::TeensyCanbus() {
  // pinMode(6, OUTPUT); digitalWrite(6, LOW); /* optional tranceiver enable pin */
}

TeensyCanbus::~TeensyCanbus() {
  // ...
}

bool TeensyCanbus::begin(LbBitrate bitrate) {
  unsigned long CAN_BAUDRATE = 250000;

  if (bitrate != lb_250kbit) {
    if (bitrate == lb_1Mbit)
      CAN_BAUDRATE = 1000000;
    else
      return false;
  }

  s_can.begin();
  s_can.setBaudRate(CAN_BAUDRATE);
  s_can.setMaxMB(16);
  s_can.enableFIFO();
  s_can.enableFIFOInterrupt();
  s_can.onReceive(s_sniff);
  s_can.mailboxStatus();

  return false;
}

void TeensyCanbus::send(uint32_t packet_id, uint8_t* bytes, int count) {
  const uint32_t extmask = 0x1FFFF800UL;

  CAN_message_t msg;

  for (uint8_t i = 0; i < count; i++) {
    msg.buf[i] = bytes[i];
  }
  msg.len = count;
  msg.id = packet_id;
  msg.flags.extended = packet_id & extmask;
  
  s_can.write(msg);
}

void TeensyCanbus::receive(const CAN_message_t &msg) {
  uint32_t packet_id = msg.id;

  if (msg.flags.remote) { // Remote transmission request, packet contains no data
    transmission_request(packet_id, msg.len); // FIXME - Check msg.len is requested transmission length
    return;
  }

  // bool bExtended = msg.flags.extended; // uses 29-bit id instead of 11-bit id

  data_received(packet_id, msg.buf, msg.len);
}

void TeensyCanbus::spin() {
  s_can.events();
}

#endif // CORE_TEENSY
