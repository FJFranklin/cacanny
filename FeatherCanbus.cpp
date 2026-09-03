/* Copyright 2026 Francis James Franklin
 * MIT license: See LICENSE file.
 */

#if defined(ADAFRUIT_FEATHER_M4_CAN)

#include <CANSAME5x.h>

#include "FeatherCanbus.hh"

static CANSAME5x s_cansame;
static FeatherCanbus s_bus(s_cansame);

void FeatherCanbus::receive_callback(int count) {
  s_bus.receive(count);
}

FeatherCanbus* FeatherCanbus::bus() {
  return &s_bus;
}

FeatherCanbus::FeatherCanbus(CANSAME5x& cansame) : m_CAN(cansame) {
  pinMode(PIN_CAN_STANDBY, OUTPUT);
  digitalWrite(PIN_CAN_STANDBY, false); // turn off STANDBY
  pinMode(PIN_CAN_BOOSTEN, OUTPUT);
  digitalWrite(PIN_CAN_BOOSTEN, true); // turn on booster

  // register the receive callback
  m_CAN.onReceive(FeatherCanbus::receive_callback);
}

FeatherCanbus::~FeatherCanbus() {
  // ...
}

bool FeatherCanbus::begin(LbBitrate bitrate) {
  if (bitrate != lb_250kbit) return false; // the feather can handle 500k, but this isn't recommended
  return m_CAN.begin(250000);
}

void FeatherCanbus::send(uint32_t packet_id, uint8_t* bytes, int count) {
  m_CAN.beginPacket(packet_id);
  for (int i = 0; i < count; i++) {
    m_CAN.write(*bytes++);
  }
  m_CAN.endPacket();
}

void FeatherCanbus::receive(int count) {
  uint32_t packet_id = m_CAN.packetId();

  if (m_CAN.packetRtr()) { // Remote transmission request, packet contains no data
    transmission_request(packet_id, m_CAN.packetDlc());
    return;
  }

  // bool bExtended = m_CAN.packetExtended(); // uses 29-bit id instead of 11-bit id

  for (int i = 0; i < count; i++) {
    uint8_t c = m_CAN.read();
    if (i < FeatherCanbus_BufferSize)
      m_buffer[i] = c;
  }
  data_received(packet_id, m_buffer, count < FeatherCanbus_BufferSize ? count : FeatherCanbus_BufferSize);
}

void FeatherCanbus::spin() {
  // ...
}

#endif // ADAFRUIT_FEATHER_M4_CAN
