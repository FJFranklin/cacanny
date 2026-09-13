/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#ifndef CaCanny_TWAI_hh
#define CaCanny_TWAI_hh

#include "CaCanny_Base.hh"

//WIRING PINS
#define CAN_TX_PIN GPIO_NUM_27
#define CAN_RX_PIN GPIO_NUM_26

namespace CaCanny {

  class TWAI : public Base {
  private:
    int m_pin_tx;
    int m_pin_rx;
  public:
    static TWAI* bus(ItemOwner<CanMessage>& store, int pin_tx = CAN_TX_PIN, int pin_rx = CAN_RX_PIN);

    TWAI(ItemOwner<CanMessage>& store, int pin_tx, int pin_rx);

    ~TWAI() { }

    bool begin(Bitrate bitrate = lb_250kbit);
    void spin();
  };

} // CaCanny

#endif /* ! CaCanny_TWAI_hh */
