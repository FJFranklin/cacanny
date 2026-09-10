/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#ifndef CaCanny_UnoR4WiFi_hh
#define CaCanny_UnoR4WiFi_hh

#include "CaCanny_Base.hh"

namespace CaCanny {

  class UnoR4WiFi : public Base {
  public:
    static UnoR4WiFi* bus(ItemOwner<CanMessage>& store);

    UnoR4WiFi(ItemOwner<CanMessage>& store);

    ~UnoR4WiFi() { }

    bool begin(Bitrate bitrate = lb_250kbit);
    void spin();
  };

} // CaCanny

#endif /* ! CaCanny_UnoR4WiFi_hh */
