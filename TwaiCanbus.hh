#ifndef TWAICANBUS_HH
#define TWAICANBUS_HH

#include "LbCanbus.hh"

class TwaiCanbus : public LbCanbus {
private:
public:
  static TwaiCanbus* bus(); // return global instance

  TwaiCanbus();

  ~TwaiCanbus();

  bool begin(LbBitrate bitrate);

  void send(uint32_t packet_id, uint8_t* bytes, int count);

  void spin();

private:
  static void receive_callback(int count);
  void receive(int count);
};

#endif /* ! TWAICANBUS_HH */
