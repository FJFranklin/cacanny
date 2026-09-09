/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#ifndef CaCanny_Minima_hh
#define CaCanny_Minima_hh

#include "CaCanny_Base.hh"

namespace CaCanny {

  class Minima : public Base {
  public:
    static Minima* bus(ItemOwner<CanMessage>& store);

    Minima(ItemOwner<CanMessage>& store);

    ~Minima() { }

    bool begin(Bitrate bitrate = lb_250kbit);
    void spin();
  };

} // CaCanny

#endif /* ! CaCanny_Minima_hh */
