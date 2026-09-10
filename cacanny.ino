/* -*- mode: c++ -*-
 * 
 * Copyright 2022-26 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#include "CaCanny_config.hh"

#include "CaCanny_Feather.hh"
#include "CaCanny_Teensy.hh"
#include "CaCanny_MCP2515.hh"

using namespace CaCanny;

static uint32_t s_default_id = 24;
static uint32_t s_accepts_id = 25;

static void id_setup() {
  /* Test arrangement is to have two Feathers talking to each other and the Teensy 4.1 talking to the MCP2515
   */
#if defined(USE_MCP2515)
  s_default_id = 29;
  s_accepts_id = 30;
#elif defined(ADAFRUIT_FEATHER_M4_CAN)
  pinMode(12, INPUT);
  if (digitalRead(12)) {
    s_default_id = 28;
    s_accepts_id = 27;
  } else {
    s_default_id = 27;
    s_accepts_id = 28;
  }
#elif defined(TEENSYDUINO)
  s_default_id = 30;
  s_accepts_id = 29;
#endif
  Serial.print("ID: TX=");
  Serial.print(s_default_id);
  Serial.print(", RX=");
  Serial.println(s_accepts_id);
}

static const uint8_t s_hex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

static uint8_t s_hex_to_int(const uint8_t* data) {
  uint8_t value = 0;
  for (int d = 0; d < 2; d++) {
    value <<= 4;
    switch(data[d]) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
       value |= data[d] - '0';
       break;
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
       value |= data[d] - 'A' + 10;
       break;
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
       value |= data[d] - 'a' + 10;
       break;
    }
  }
  return value;
}

class App : public Timer, public Handler {
private:
  ItemOwner<CanMessage>& m_store;
  Base* m_bus;
  uint8_t m_last_ping_sent;
  uint32_t m_ping_count;
  uint32_t m_pong_count;
  LED* m_led;
public:
  App(ItemOwner<CanMessage>& store, Base* bus) :
    m_store(store),
    m_bus(bus),
    m_last_ping_sent(0),
    m_ping_count(0),
    m_pong_count(0)
  {
    m_led = LED::onboard_LED();
  }
  ~App() {
    // ...
  }

  void every_milli() { // runs once a millisecond, on average
    // ...
  }

  void every_10ms() { // runs once every 10ms, on average
    // ...
    send_ping();
  }

  void every_tenth(int tenth) { // runs once every tenth of a second, where tenth = 0..9
    m_led->blink((tenth == 0) || (tenth == 3));
  }

  void every_second() { // runs once every second
    uint32_t received;
    uint32_t lost;
    m_bus->received_message_stats(received, lost);
    Serial.print("lost ");
    Serial.print(lost);
    Serial.print("/");
    Serial.print(received);
    Serial.print(", lost pings = ");
    Serial.println(m_ping_count - m_pong_count);
  }

  void send_ping() {
    ++m_last_ping_sent;
    ++m_ping_count;

    CanMessage* msg = m_store.pop();
    if (!msg) return; // no messages in store

    msg->data_frame(s_default_id, 8);
    memcpy(msg->buffer, "ping: ", 6);

    msg->buffer[6] = s_hex[m_last_ping_sent >> 4];
    msg->buffer[7] = s_hex[m_last_ping_sent & 0x0F];
    m_bus->send(msg);
  }
  void ping_received(uint32_t packet_id, uint8_t seq_no) { // send pong
    CanMessage* msg = m_store.pop();
    if (!msg) return; // no messages in store

    msg->data_frame(s_default_id, 8);
    memcpy(msg->buffer, "pong: ", 6);

    msg->buffer[6] = s_hex[seq_no >> 4];
    msg->buffer[7] = s_hex[seq_no & 0x0F];
    m_bus->send(msg);
  }
  void pong_received(uint32_t packet_id, uint8_t seq_no) {
    if (seq_no != m_last_ping_sent) {
      Serial.print(seq_no);
      Serial.print(" != ");
      Serial.println(m_last_ping_sent);
    } else {
      ++m_pong_count; // count matching pongs only
    }
  }
  void tick() {
    m_bus->spin();

    CanMessage* msg = m_bus->next(); // check for queued incoming messages
    if (!msg) return;

    if (msg->length == 8) {
      if (memcmp(msg->buffer, "ping: ", 6) == 0) {
        ping_received(msg->id, s_hex_to_int(msg->buffer + 6));
      }
      if (memcmp(msg->buffer, "pong: ", 6) == 0) {
        pong_received(msg->id, s_hex_to_int(msg->buffer + 6));
      }
    }
    msg->return_to_owner();
  }

  bool will_accept_id(uint32_t id) {
    /* This is called from within an interrupt, so be quick and don't play with hardware
     */
    return id == s_accepts_id; // only care about one incoming ID
  }
};

MessageStore<16> store;

void setup() {
  Serial.begin(115200);
  delay(2000);

  id_setup(); // see who we are and who we're listening to

#if defined(USE_MCP2515)
  MCP2515* bus = MCP2515::bus(store);
#elif defined(ADAFRUIT_FEATHER_M4_CAN)
  Feather* bus = Feather::bus(store);
#elif defined(TEENSYDUINO)
  Teensy* bus = Teensy::bus(store);
#endif
  if (bus) {
    App app(store, bus);

    bus->set_handler(&app);
    if (bus->begin()) {
      Serial.println("Have canbus, starting app...");
      app.run();
    } else {
      Serial.println("Canbus error.");
      LED::onboard_LED()->error(1, 1, 2);
    }
  } else {
    Serial.println("No canbus?");
    LED::onboard_LED()->error(1, 1, 1);
  }

  Serial.println("Application exited.");
  LED::onboard_LED()->error(1, 1, 3);
}

void loop() {
  // unreachable
}
