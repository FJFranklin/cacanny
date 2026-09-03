/* Copyright 2026 Francis James Franklin
 * MIT license: See LICENSE file.
 */

#ifndef FEATHERCANBUS_HH
#define FEATHERCANBUS_HH

#include "LbCanbus.hh"

class CANSAME5x;

const int FeatherCanbus_BufferSize = 8; // 64 bit data frame

class FeatherCanbus : public LbCanbus {
private:
  uint8_t m_buffer[FeatherCanbus_BufferSize];

  CANSAME5x& m_CAN;
public:
  static FeatherCanbus* bus(); // return global instance

  FeatherCanbus(CANSAME5x& cansame);

  ~FeatherCanbus();

  bool begin(LbBitrate bitrate);

  void send(uint32_t packet_id, uint8_t* bytes, int count);

  void spin();

private:
  static void receive_callback(int count);
  void receive(int count);
};

#endif /* ! FEATHERCANBUS_HH */
