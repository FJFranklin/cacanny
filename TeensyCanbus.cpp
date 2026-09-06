/* Copyright 2026 Francis James Franklin
 * MIT license: See LICENSE file.
 */

#if defined(TEENSYDUINO)

#include <FlexCAN_T4.h>

#include "TeensyCanbus.hh"

static FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> s_can1;
static FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> s_can2;
static FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> s_can3;

static Teensy4Canbus s_bus1(Teensy4Canbus_BusNumber::tc_bn_1, &s_can1);
static Teensy4Canbus s_bus2(Teensy4Canbus_BusNumber::tc_bn_2, &s_can2);
static Teensy4Canbus s_bus3(Teensy4Canbus_BusNumber::tc_bn_3, &s_can3);

void Teensy4Canbus::bus1_sniff(const CAN_message_t& msg) {
  s_bus1.receive(msg);
}

void Teensy4Canbus::bus2_sniff(const CAN_message_t& msg) {
  s_bus2.receive(msg);
}

void Teensy4Canbus::bus3_sniff(const CAN_message_t& msg) {
  s_bus3.receive(msg);
}

Teensy4Canbus* Teensy4Canbus::bus(Teensy4Canbus_BusNumber can_no) {
  Teensy4Canbus* instance = 0;

  switch (can_no) {
  case Teensy4Canbus_BusNumber::tc_bn_1:
    instance = &s_bus1;
    break;
  case Teensy4Canbus_BusNumber::tc_bn_2:
    instance = &s_bus2;
    break;
  case Teensy4Canbus_BusNumber::tc_bn_3:
    instance = &s_bus3;
    break;
  }
  return instance;
}

Teensy4Canbus::Teensy4Canbus(Teensy4Canbus_BusNumber can_no, FlexCAN_T4_Base* can_base) : m_can_no(can_no), m_can_base(can_base) {
  // pinMode(6, OUTPUT); digitalWrite(6, LOW); /* optional tranceiver enable pin */
}

Teensy4Canbus::~Teensy4Canbus() {
  // ...
}

bool Teensy4Canbus::begin(LbBitrate bitrate) {
  unsigned long CAN_BAUDRATE = 250000;

  if (bitrate != lb_250kbit) {
    if (bitrate == lb_1Mbit)
      CAN_BAUDRATE = 1000000;
    else
      return false;
  }

  switch (m_can_no) {
  case Teensy4Canbus_BusNumber::tc_bn_1:
    s_can1.begin();
    s_can1.setBaudRate(CAN_BAUDRATE);
    s_can1.setMaxMB(16);
    s_can1.enableFIFO();
    s_can1.enableFIFOInterrupt();
    s_can1.onReceive(bus1_sniff);
    break;
  case Teensy4Canbus_BusNumber::tc_bn_2:
    s_can2.begin();
    s_can2.setBaudRate(CAN_BAUDRATE);
    s_can2.setMaxMB(16);
    s_can2.enableFIFO();
    s_can2.enableFIFOInterrupt();
    s_can2.onReceive(bus2_sniff);
    break;
  case Teensy4Canbus_BusNumber::tc_bn_3:
    s_can3.begin();
    s_can3.setBaudRate(CAN_BAUDRATE);
    s_can3.setMaxMB(16);
    s_can3.enableFIFO();
    s_can3.enableFIFOInterrupt();
    s_can3.onReceive(bus3_sniff);
    break;
  }
  return false;
}

void Teensy4Canbus::send(uint32_t packet_id, uint8_t* bytes, int count) {
  const uint32_t extmask = 0x1FFFF800UL;

  CAN_message_t msg;

  for (uint8_t i = 0; i < count; i++) {
    msg.buf[i] = bytes[i];
  }
  msg.len = count;
  msg.id = packet_id;
  msg.flags.extended = packet_id & extmask;
  
  m_can_base->write(msg);
}

void Teensy4Canbus::receive(const CAN_message_t &msg) {
  uint32_t packet_id = msg.id;

  if (msg.flags.remote) { // Remote transmission request, packet contains no data
    transmission_request(packet_id, msg.len); // FIXME - Check msg.len is requested transmission length
    return;
  }

  // bool bExtended = msg.flags.extended; // uses 29-bit id instead of 11-bit id

  data_received(packet_id, msg.buf, msg.len);
}

void Teensy4Canbus::spin() {
  m_can_base->events();
}

#endif // TEENSYDUINO
