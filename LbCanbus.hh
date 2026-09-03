/* Copyright 2026 Francis James Franklin
 * MIT license: See LICENSE file.
 */

#ifndef LBCANBUS_HH
#define LBCANBUS_HH

enum LbBitrate {
  lb_250kbit = 0,
  lb_1Mbit
};

class LbCanbus_RequestHandler {
public:
  virtual ~LbCanbus_RequestHandler() {
    // ...
  }
  virtual void canbus_transmission_request(uint32_t packet_id, int length) = 0;
};

class LbCanbus_DataHandler {
public:
  virtual ~LbCanbus_DataHandler() {
    // ...
  }
  virtual void canbus_data_received(uint32_t packet_id, const uint8_t* data, int length) = 0;
};

class LbCanbus {
private:
  LbCanbus_RequestHandler* m_request_handler;
  LbCanbus_DataHandler* m_data_handler;

protected:
  inline void transmission_request(uint32_t packet_id, int length) {
    if (m_request_handler)
      m_request_handler->canbus_transmission_request(packet_id, length);
  }
  inline void data_received(uint32_t packet_id, const uint8_t* data, int length) {
    if (m_data_handler)
      m_data_handler->canbus_data_received(packet_id, data, length);
  }

public:
  inline void set_request_handler(LbCanbus_RequestHandler* handler) {
    m_request_handler = handler;
  }
  inline void set_data_handler(LbCanbus_DataHandler* handler) {
    m_data_handler = handler;
  }

  LbCanbus() : m_request_handler(0), m_data_handler(0) {
    // ...
  }
  virtual ~LbCanbus() {
    // ...
  }

  virtual bool begin(LbBitrate bitrate = lb_250kbit) = 0;

  virtual void send(uint32_t packet_id, uint8_t* bytes, int count) = 0;

  virtual void spin() = 0;
};

#endif /* ! LBCANBUS_HH */
