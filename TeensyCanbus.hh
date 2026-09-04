/* Copyright 2026 Francis James Franklin
 * MIT license: See LICENSE file.
 */

#ifndef TEENSYCANBUS_HH
#define TEENSYCANBUS_HH

#include "LbCanbus.hh"

class CAN_message_t;

class TeensyCanbus : public LbCanbus {
private:
public:
  static TeensyCanbus* bus(); // return global instance

  TeensyCanbus();

  ~TeensyCanbus();

  bool begin(LbBitrate bitrate);

  void send(uint32_t packet_id, uint8_t* bytes, int count);

  void spin();

private:
  void receive(const CAN_message_t& msg);
};

#endif /* ! TEENSYCANBUS_HH */
