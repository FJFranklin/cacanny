#include <Adafruit_MCP2515.h>

#include "Ada2515Canbus.hh"

#define CS_PIN    8
#define INT_PIN   3

static Adafruit_MCP2515 s_mcp(CS_PIN);
static Ada2515Canbus s_bus(s_mcp);

void Ada2515Canbus::receive_callback(int count) {
  s_bus.receive(count);
}

Ada2515Canbus* Ada2515Canbus::bus() {
  return &s_bus;
}

Ada2515Canbus::Ada2515Canbus(Adafruit_MCP2515& mcp) : m_mcp(mcp) {
  // ...
}

Ada2515Canbus::~Ada2515Canbus() {
  // ...
}

bool Ada2515Canbus::begin(LbBitrate bitrate) {
  unsigned long CAN_BAUDRATE = 250000;

  if (bitrate != lb_250kbit) {
    if (bitrate == lb_1Mbit)
      CAN_BAUDRATE = 1000000;
    else
      return false;
  }
  if (!m_mcp.begin(CAN_BAUDRATE)) {
    return false;
  }

  // register the receive callback
  m_mcp.onReceive(INT_PIN, receive_callback);

  return true;
}

void Ada2515Canbus::send(uint32_t packet_id, uint8_t* bytes, int count) {
  const uint32_t extmask = 0x1FFFF800UL;

  if (packet_id & extmask)
    m_mcp.beginExtendedPacket(packet_id);
  else
    m_mcp.beginPacket(packet_id);

  for (int i = 0; i < count; i++)
    m_mcp.write(bytes[i]);
  m_mcp.endPacket();
}

void Ada2515Canbus::receive(int count) {
  uint32_t packet_id = m_mcp.packetId();

  if (m_mcp.packetRtr()) { // Remote transmission request, packet contains no data
    transmission_request(packet_id, m_mcp.packetDlc());
    return;
  }

  // bool bExtended = m_mcp.packetExtended(); // uses 29-bit id instead of 11-bit id

  for (int i = 0; i < count; i++) {
    uint8_t c = m_mcp.read();
    if (i < Ada2515Canbus_BufferSize)
      m_buffer[i] = c;
  }
  data_received(packet_id, m_buffer, count < Ada2515Canbus_BufferSize ? count : Ada2515Canbus_BufferSize);
}

void Ada2515Canbus::spin() {
  // ...
}
