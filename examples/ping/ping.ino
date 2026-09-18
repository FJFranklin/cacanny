/* -*- mode: c++ -*-
 * 
 * Copyright 2022-26 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#include "CaCanny_config.hh"

/* Uncomment one of the following if building on Teensy
 */
//#define TEST_CAN1_MCP2515_250k  1
//#define TEST_CAN2_MCP2515_1000k 1
#define TEST_CAN1_CAN2_250k     1
//#define TEST_CAN1_CAN2_1000k    1

#include "CaCanny_Feather.hh"
#include "CaCanny_Teensy.hh"
#include "CaCanny_MCP2515.hh"
#include "CaCanny_TWAI.hh"
#include "CaCanny_UnoR4WiFi.hh"

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

using namespace CaCanny;

/* The application will have a linked list of handlers for each canbus
 */
class BusInfo : public Handler {
private:
  ItemOwner<CanMessage>& m_store;

  Base*    m_bus;
  uint32_t m_default_id;
  uint32_t m_accepts_id;
  BusInfo* m_next;
public:
  uint8_t  m_last_ping_sent;
  uint32_t m_ping_count;
  uint32_t m_pong_count;

  BusInfo(ItemOwner<CanMessage>& store, Base* bus, uint32_t default_id, uint32_t accepts_id) :
    m_store(store),
    m_bus(bus),
    m_default_id(default_id),
    m_accepts_id(accepts_id),
    m_next(0),
    m_last_ping_sent(0),
    m_ping_count(0),
    m_pong_count(0)
  {
    // ...
  }
  ~BusInfo() {
    // ...
  }
  BusInfo* next() {
    return m_next;
  }
  void append_to_list(BusInfo* businfo) {
    if (m_next)
      m_next->append_to_list(businfo);
    else
      m_next = businfo;
  }

  bool will_accept_id(uint32_t id) {
    /* This is called from within an interrupt, so be quick and don't play with hardware
     */
    return id == m_accepts_id; // only care about one incoming ID
  }
  void spin() {
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

  void send_ping() {
    CanMessage* msg = m_store.pop();
    if (!msg) return; // no messages in store

    m_last_ping_sent++;
    m_ping_count++;

    msg->data_frame(m_default_id, 8);
    memcpy(msg->buffer, "ping: ", 6);

    msg->buffer[6] = s_hex[m_last_ping_sent >> 4];
    msg->buffer[7] = s_hex[m_last_ping_sent & 0x0F];
    m_bus->send(msg);
  }
  void ping_received(uint32_t packet_id, uint8_t seq_no) { // send pong
    CanMessage* msg = m_store.pop();
    if (!msg) return; // no messages in store

    msg->data_frame(m_default_id, 8);
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

  void print_summary() {
    uint32_t received;
    uint32_t lost;
    m_bus->received_message_stats(received, lost);

    Serial.print("Bus '");
    Serial.print(m_bus->backend());
    Serial.print("' lost messages: ");
    Serial.print(lost);
    Serial.print("/");
    Serial.print(received);
    Serial.print(", lost pings = ");
    Serial.println(m_ping_count - m_pong_count);
  }
};

class App : public Timer {
private:
  ItemOwner<CanMessage>& m_store;
  BusInfo* m_businfo;
  LED* m_led;
public:
  App(ItemOwner<CanMessage>& store) :
    m_store(store),
    m_businfo(0)
  {
    m_led = LED::onboard_LED();
  }
  ~App() {
    // ...
  }
  void add(BusInfo* businfo) {
    if (m_businfo)
      m_businfo->append_to_list(businfo);
    else
      m_businfo = businfo;
  }

  void every_milli() { // runs once a millisecond, on average
#ifdef TEST_CAN1_CAN2_1000k
    BusInfo* bi = m_businfo;
    while (bi) {
      bi->send_ping();
      bi = bi->next();
    }
#endif
  }

  void every_10ms() { // runs once every 10ms, on average
#ifndef TEST_CAN1_CAN2_1000k
    BusInfo* bi = m_businfo;
    while (bi) {
      bi->send_ping();
      bi = bi->next();
    }
#endif
  }

  void every_tenth(int tenth) { // runs once every tenth of a second, where tenth = 0..9
#ifdef TEST_CAN1_CAN2_1000k
    m_led->blink((tenth == 0) || (tenth == 2) || (tenth == 4));
#else
    m_led->blink((tenth == 0) || (tenth == 3));
#endif
  }

  void every_second() { // runs once every second
    BusInfo* bi = m_businfo;
    while (bi) {
      bi->print_summary();
      bi = bi->next();
    }
  }

  void tick() {
    BusInfo* bi = m_businfo;
    while (bi) {
      bi->spin();
      bi = bi->next();
    }
  }
};

MessageStore<16> store;

void setup() {
  Serial.begin(115200);
  delay(2000);

  App app(store);

#if defined(ADAFRUIT_FEATHER_M4_CAN)
  /* The original test setup has two Feather M4 CAN devices talking to each other at 250kbit.
   * The state of digital pin 12 is used to determine which of the two devices this is.
   */
  pinMode(12, INPUT);
  uint32_t default_id = 27;
  uint32_t accepts_id = 28;
  if (digitalRead(12)) {
    default_id = 28;
    accepts_id = 27;
  }

  Feather* bus = Feather::bus(store);
  if (!bus) {
    Serial.println("No canbus?");
    LED::onboard_LED()->error(1, 1, 1);
    // ~~ (unreached) ~~
  }

  BusInfo BI(store, bus, default_id, accepts_id);
  bus->set_handler(&BI);

  if (!bus->begin()) {
    Serial.println("Canbus error.");
    LED::onboard_LED()->error(1, 1, 2);
    // ~~ (unreached) ~~
  }

  app.add(&BI);

  Serial.println("Have Feather M4 CAN bus @ 250kbit");

#elif defined(TEENSYDUINO)
  /* The alternative setup has a Teensy 4.1 with transceivers connected to CAN1 and CAN2
   * and an MCP2515 connected to SPI.
   * There are two possible tests with this: (a) CAN1 at 250k, MCP at 250k; (b) CAN1 and CAN2 at 250k;
   * and (c) CAN2 at 1Mbit, MCP at 1Mbit.
   */
#if defined(TEST_CAN1_MCP2515_250k)

  Teensy* bus1 = Teensy::bus(store, TeensyBus::tc_bn_1);
  if (!bus1) {
    Serial.println("No Teensy CAN(1) bus?");
    LED::onboard_LED()->error(2, 1, 1);
    // ~~ (unreached) ~~
  }

  BusInfo BI1(store, bus1, 29 /* default_id */, 30 /* accepts_id */);
  bus1->set_handler(&BI1);

  if (!bus1->begin(Bitrate::lb_250kbit)) {
    Serial.println("Teensy CAN(1) bus error.");
    LED::onboard_LED()->error(2, 1, 2);
    // ~~ (unreached) ~~
  }

  app.add(&BI1);
  Serial.println("Have Teensy CAN(1) bus @ 250kbit");
  delay(100);

  MCP2515* bus2 = MCP2515::bus(store);
  if (!bus2) {
    Serial.println("No MCP2515 bus?");
    LED::onboard_LED()->error(2, 2, 1);
    // ~~ (unreached) ~~
  }

  BusInfo BI2(store, bus2, 30 /* default_id */, 29 /* accepts_id */);
  bus2->set_handler(&BI2);

  if (!bus2->begin(Bitrate::lb_250kbit)) {
    Serial.println("MCP2515 bus error.");
    LED::onboard_LED()->error(2, 2, 2);
    // ~~ (unreached) ~~
  }

  app.add(&BI2);
  Serial.println("Have MCP2515 bus @ 250kbit");

#elif defined(TEST_CAN1_CAN2_250k)

  Teensy* bus1 = Teensy::bus(store, TeensyBus::tc_bn_1);
  if (!bus1) {
    Serial.println("No Teensy CAN(1) bus?");
    LED::onboard_LED()->error(3, 1, 1);
    // ~~ (unreached) ~~
  }

  BusInfo BI1(store, bus1, 29 /* default_id */, 30 /* accepts_id */);
  bus1->set_handler(&BI1);

  if (!bus1->begin(Bitrate::lb_250kbit)) {
    Serial.println("Teensy CAN(1) bus error.");
    LED::onboard_LED()->error(3, 1, 2);
    // ~~ (unreached) ~~
  }

  app.add(&BI1);
  Serial.println("Have Teensy CAN(1) bus @ 250kbit");
  delay(100);

  Teensy* bus2 = Teensy::bus(store, TeensyBus::tc_bn_2);
  if (!bus2) {
    Serial.println("No Teensy CAN(2) bus?");
    LED::onboard_LED()->error(3, 2, 1);
    // ~~ (unreached) ~~
  }

  BusInfo BI2(store, bus2, 30 /* default_id */, 29 /* accepts_id */);
  bus2->set_handler(&BI2);

  if (!bus2->begin(Bitrate::lb_250kbit)) {
    Serial.println("Teensy CAN(2) bus error.");
    LED::onboard_LED()->error(3, 2, 2);
    // ~~ (unreached) ~~
  }

  app.add(&BI2);
  Serial.println("Have Teensy CAN(2) bus @ 250kbit");

#elif defined(TEST_CAN2_MCP2515_1000k)

  Teensy* bus1 = Teensy::bus(store, TeensyBus::tc_bn_2);
  if (!bus1) {
    Serial.println("No Teensy CAN(1) bus?");
    LED::onboard_LED()->error(4, 1, 1);
    // ~~ (unreached) ~~
  }

  BusInfo BI1(store, bus1, 29 /* default_id */, 30 /* accepts_id */);
  bus1->set_handler(&BI1);

  if (!bus1->begin(Bitrate::lb_1Mbit)) {
    Serial.println("Teensy CAN(2) bus error.");
    LED::onboard_LED()->error(4, 1, 2);
    // ~~ (unreached) ~~
  }

  app.add(&BI1);
  Serial.println("Have Teensy CAN(2) bus @ 1Mbit");
  delay(100);

  MCP2515* bus2 = MCP2515::bus(store);
  if (!bus2) {
    Serial.println("No MCP2515 bus?");
    LED::onboard_LED()->error(4, 2, 1);
    // ~~ (unreached) ~~
  }

  BusInfo BI2(store, bus2, 30 /* default_id */, 29 /* accepts_id */);
  bus2->set_handler(&BI2);

  if (!bus2->begin(Bitrate::lb_1Mbit)) {
    Serial.println("MCP2515 bus error.");
    LED::onboard_LED()->error(4, 2, 2);
    // ~~ (unreached) ~~
  }

  app.add(&BI2);
  Serial.println("Have MCP2515 bus @ 1Mbit");

#elif defined(TEST_CAN1_CAN2_1000k)

  Teensy* bus1 = Teensy::bus(store, TeensyBus::tc_bn_1);
  if (!bus1) {
    Serial.println("No Teensy CAN(1) bus?");
    LED::onboard_LED()->error(5, 1, 1);
    // ~~ (unreached) ~~
  }

  BusInfo BI1(store, bus1, 29 /* default_id */, 30 /* accepts_id */);
  bus1->set_handler(&BI1);

  if (!bus1->begin(Bitrate::lb_1Mbit)) {
    Serial.println("Teensy CAN(1) bus error.");
    LED::onboard_LED()->error(5, 1, 2);
    // ~~ (unreached) ~~
  }

  app.add(&BI1);
  Serial.println("Have Teensy CAN(1) bus @ 1Mbit");
  delay(100);

  Teensy* bus2 = Teensy::bus(store, TeensyBus::tc_bn_2);
  if (!bus2) {
    Serial.println("No Teensy CAN(2) bus?");
    LED::onboard_LED()->error(5, 2, 1);
    // ~~ (unreached) ~~
  }

  BusInfo BI2(store, bus2, 30 /* default_id */, 29 /* accepts_id */);
  bus2->set_handler(&BI2);

  if (!bus2->begin(Bitrate::lb_1Mbit)) {
    Serial.println("Teensy CAN(2) bus error.");
    LED::onboard_LED()->error(5, 2, 2);
    // ~~ (unreached) ~~
  }

  app.add(&BI2);
  Serial.println("Have Teensy CAN(2) bus @ 1Mbit");

#endif // Teensy tests
#elif defined(ESP_PLATFORM)

  TWAI* bus1 = TWAI::bus(store);
  if (!bus1) {
    Serial.println("No TWAI bus?");
    LED::onboard_LED()->error(6, 1, 1);
    // ~~ (unreached) ~~
  }

  BusInfo BI1(store, bus1, 29 /* default_id */, 30 /* accepts_id */);
  bus1->set_handler(&BI1);

  if (!bus1->begin(Bitrate::lb_250kbit)) {
    Serial.println("TWAI bus error.");
    LED::onboard_LED()->error(6, 1, 2);
    // ~~ (unreached) ~~
  }

  app.add(&BI1);
  Serial.println("Have TWAI bus @ 250kbit");
  delay(100);

#elif defined(ARDUINO_UNOR4_WIFI)
  /* Variation on the original test setup but two Uno R4 WiFi devices talking to each other at 250kbit.
   * The state of digital pin 12 is used to determine which of the two devices this is.
   */
  pinMode(12, INPUT);
  uint32_t default_id = 31;
  uint32_t accepts_id = 32;
  if (digitalRead(12)) {
    default_id = 32;
    accepts_id = 31;
  }

  UnoR4WiFi* bus = UnoR4WiFi::bus(store);
  if (!bus) {
    Serial.println("No canbus?");
    LED::onboard_LED()->error(7, 1, 1);
    // ~~ (unreached) ~~
  }

  BusInfo BI(store, bus, default_id, accepts_id);
  bus->set_handler(&BI);

  if (!bus->begin()) {
    Serial.println("Canbus error.");
    LED::onboard_LED()->error(7, 1, 2);
    // ~~ (unreached) ~~
  }

  app.add(&BI);

  Serial.println("Have Uno R4 WiFi CAN bus @ 250kbit");

#endif

  Serial.println("Starting app...");
  delay(1000);
  app.run();

  Serial.println("Application exited.");
  LED::onboard_LED()->error(1, 1, 3);
}

void loop() {
  // unreachable
}
