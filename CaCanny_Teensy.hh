/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#ifndef CaCanny_Teensy_hh
#define CaCanny_Teensy_hh

#include "CaCanny_Base.hh"

class CAN_message_t;
class FlexCAN_T4_Base;

namespace CaCanny {

  enum TeensyBus {
    tc_bn_1,
    tc_bn_2,
    tc_bn_3
  };

  class Teensy : public Base {
  private:
    TeensyBus m_can_no;
    FlexCAN_T4_Base* m_can_base;
  public:
    static Teensy* bus(ItemOwner<CanMessage>& store, TeensyBus can_no = TeensyBus::tc_bn_1);

    Teensy(ItemOwner<CanMessage>& store, FlexCAN_T4_Base* can_base, TeensyBus can_no);

    ~Teensy() { }

    bool begin(Bitrate bitrate = lb_250kbit);
    void spin();

  private:
    static void bus1_sniff(const CAN_message_t& msg);
    static void bus2_sniff(const CAN_message_t& msg);
    static void bus3_sniff(const CAN_message_t& msg);
    void receive(const CAN_message_t& msg);
  };

} // CaCanny

#endif /* ! CaCanny_Teensy_hh */
