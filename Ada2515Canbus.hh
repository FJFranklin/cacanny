/* Copyright 2026 Francis James Franklin
 * MIT license: See LICENSE file.
 */

#ifndef ADA2515CANBUS_HH
#define ADA2515CANBUS_HH

#include "LbCanbus.hh"

class Adafruit_MCP2515;

const int Ada2515Canbus_BufferSize = 8; // 64 bit data frame

class Ada2515Canbus : public LbCanbus {
private:
  uint8_t m_buffer[Ada2515Canbus_BufferSize];

  Adafruit_MCP2515& m_mcp;
public:
  static Ada2515Canbus* bus(); // return global instance

  Ada2515Canbus(Adafruit_MCP2515& mcp);

  ~Ada2515Canbus();

  bool begin(LbBitrate bitrate);

  void send(uint32_t packet_id, uint8_t* bytes, int count);

  void spin();

private:
  static void receive_callback(int count);
  void receive(int count);
};

#endif /* ! ADA2515CANBUS_HH */
