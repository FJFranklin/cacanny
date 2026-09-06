/* Copyright 2026 Francis James Franklin
 * MIT license: See LICENSE file.
 */

#ifndef TEENSY4CANBUS_HH
#define TEENSY4CANBUS_HH

#include "LbCanbus.hh"

class CAN_message_t;
class FlexCAN_T4_Base;

enum Teensy4Canbus_BusNumber {
  tc_bn_1,
  tc_bn_2,
  tc_bn_3
};

class Teensy4Canbus : public LbCanbus {
private:
  Teensy4Canbus_BusNumber m_can_no;
  FlexCAN_T4_Base* m_can_base;

public:
  static Teensy4Canbus* bus(Teensy4Canbus_BusNumber can_no = Teensy4Canbus_BusNumber::tc_bn_1); // return global instance

  Teensy4Canbus(Teensy4Canbus_BusNumber can_no, FlexCAN_T4_Base* can_base);

  ~Teensy4Canbus();

  bool begin(LbBitrate bitrate);

  void send(uint32_t packet_id, uint8_t* bytes, int count);

  void spin();

private:
  static void bus1_sniff(const CAN_message_t& msg);
  static void bus2_sniff(const CAN_message_t& msg);
  static void bus3_sniff(const CAN_message_t& msg);
  void receive(const CAN_message_t& msg);
};

#endif /* ! TEENSY4CANBUS_HH */
