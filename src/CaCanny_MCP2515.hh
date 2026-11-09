/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#ifndef CaCanny_MCP2515_hh
#define CaCanny_MCP2515_hh

#include "CaCanny_Base.hh"

#if ENABLE_MCP2515_Adafruit
class Adafruit_MCP2515;
#endif
#if ENABLE_MCP2515_autowp
class MCP2515;
struct can_frame;
#endif

namespace CaCanny {

  class CC_MCP2515 : public Base {
  private:
#if ENABLE_MCP2515_Adafruit
    Adafruit_MCP2515* m_mcp;
#endif
#if ENABLE_MCP2515_autowp
    MCP2515* m_mcp;
    can_frame* m_msg;
#endif
    int m_pin_chip_select;
    int m_pin_interrupt;
  public:
    static CC_MCP2515* bus(ItemOwner<CanMessage>& store, int pin_chip_select = 10, int pin_interrupt = 13);

    CC_MCP2515(ItemOwner<CanMessage>& store, int pin_chip_select, int pin_interrupt);

    ~CC_MCP2515();

    bool begin(Bitrate bitrate = lb_250kbit);
    void spin();

  private:
    static void receive_callback(int count);
    void receive(int count);
  };

} // CaCanny

#endif /* ! CaCanny_MCP2515_hh */
