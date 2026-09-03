#ifndef R4CANBUS_HH
#define R4CANBUS_HH

#include "LbCanbus.hh"

class R4Canbus : public LbCanbus {
public:
  static R4Canbus* bus(); // return global instance

  R4Canbus();

  ~R4Canbus();

  bool begin(LbBitrate bitrate);

  void send(uint32_t packet_id, uint8_t* bytes, int count);

  void spin();
};

#endif /* ! R4CANBUS_HH */
