/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#ifndef CaCanny_Feather_hh
#define CaCanny_Feather_hh

#include "CaCanny_Base.hh"

class CANSAME5x;

namespace CaCanny {

  class Feather : public Base {
  private:
    CANSAME5x& m_CAN;
  public:
    static Feather* bus(ItemOwner<CanMessage>& store);

    Feather(ItemOwner<CanMessage>& store, CANSAME5x& cansame);

    ~Feather() { }

    bool begin(Bitrate bitrate = lb_250kbit);
    void spin();

  private:
    static void receive_callback(int count);
    void receive(int count);
  };

} // CaCanny

#endif /* ! CaCanny_Feather_hh */
