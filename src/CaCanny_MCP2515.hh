/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#ifndef CaCanny_MCP2515_hh
#define CaCanny_MCP2515_hh

#include "CaCanny_Base.hh"

class Adafruit_MCP2515;

namespace CaCanny {

  class MCP2515 : public Base {
  private:
    Adafruit_MCP2515* m_mcp;
    int m_pin_interrupt;
  public:
    static MCP2515* bus(ItemOwner<CanMessage>& store, int pin_chip_select = 10, int pin_interrupt = 13);

    MCP2515(ItemOwner<CanMessage>& store, int pin_chip_select, int pin_interrupt);

    ~MCP2515();

    bool begin(Bitrate bitrate = lb_250kbit);
    void spin();

  private:
    static void receive_callback(int count);
    void receive(int count);
  };

} // CaCanny

#endif /* ! CaCanny_MCP2515_hh */
