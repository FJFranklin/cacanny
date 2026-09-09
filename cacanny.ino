/* -*- mode: c++ -*-
 * 
 * Copyright 2022-26 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#include "CaCanny_Feather.hh"

using namespace CaCanny;

const uint32_t default_id = 27;

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
  uint8_t m_ping_seq_no;
  LED* m_led;
public:
  App(ItemOwner<CanMessage>& store, Base* bus) : m_store(store), m_bus(bus), m_ping_seq_no(0) {
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
    Serial.println(received);
  }

  void send_ping() {
    CanMessage* msg = m_store.pop();
    if (!msg) return; // no messages in store

    msg->data_frame(default_id, 8);
    memcpy(msg->buffer, "ping: ", 6);

    msg->buffer[6] = s_hex[m_ping_seq_no >> 4];
    msg->buffer[7] = s_hex[m_ping_seq_no & 0x0F];
    m_bus->send(msg);

    ++m_ping_seq_no;
    // Serial.println("ping sent");
  }
  void ping_received(uint32_t packet_id, uint8_t seq_no) { // send pong
    CanMessage* msg = m_store.pop();
    if (!msg) return; // no messages in store

    msg->data_frame(default_id, 8);
    memcpy(msg->buffer, "pong: ", 6);

    msg->buffer[6] = s_hex[seq_no >> 4];
    msg->buffer[7] = s_hex[seq_no & 0x0F];
    m_bus->send(msg);

    // Serial.print(" <<");
  }
  void pong_received(uint32_t packet_id, uint8_t seq_no) {
    if (m_ping_seq_no == (uint8_t) (seq_no + 1)) {
      // Serial.print(" OK");
    } else {
      Serial.print(" ");
      Serial.print(seq_no);
      Serial.print(" != ");
      Serial.println((uint8_t) (m_ping_seq_no - 1));
    }
  }
  void tick() {
    m_bus->spin();

    CanMessage* msg = m_bus->next(); // check for queued incoming messages
    if (!msg) return;

    /* if (msg->remote)
      Serial.print("r(");
    else
      Serial.print("d(");
    if (msg->extended)
      Serial.print("e) [");
    else
      Serial.print("s) [");

    Serial.print(msg->id);
    Serial.print("]");

    if (!msg->remote) {
      Serial.print(" '");
      for (uint8_t i = 0; i < msg->length; i++)
        Serial.write(msg->buffer[i]);
      Serial.print("'");
    }*/

    if (msg->length == 8) {
      if (memcmp(msg->buffer, "ping: ", 6) == 0) {
        //Serial.print(" ping!");
        ping_received(msg->id, s_hex_to_int(msg->buffer + 6));
      }
      if (memcmp(msg->buffer, "pong: ", 6) == 0) {
        //Serial.print(" pong!");
        pong_received(msg->id, s_hex_to_int(msg->buffer + 6));
      }
    }

    //Serial.println();

    msg->return_to_owner();
  }

  bool will_accept_id(uint32_t id) {
    /* This is called from within an interrupt, so be quick and don't play with hardware
     */
    return true;
  }
};

MessageStore<16> store;

void setup() {
  Serial.begin(115200);

  Feather* bus = Feather::bus(store);
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
