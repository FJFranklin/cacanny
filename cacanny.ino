// -*-c++-*-

/* Copyright 2026 Francis James Franklin
 * MIT license: See LICENSE file.
 */

#if defined(ADAFRUIT_FEATHER_M4_CAN)
#include <Adafruit_NeoPixel.h>

static inline void s_blink(bool bOn, bool bError = false) {
  static bool bInit = true;

  static Adafruit_NeoPixel s_led(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

  if (bInit) {
    s_led.begin();
    s_led.setPixelColor(0, s_led.Color(0, 255, 0));
    bInit = false;
  }
  if (bError) { // if bError is set, switch color to red
    s_led.setPixelColor(0, s_led.Color(255, 0, 0));
  }
  s_led.setBrightness(bOn ? 31 : 1);
  s_led.show();
}
#else
static inline void s_blink(bool bOn, bool bError = false) {
  (void) bError;
  digitalWrite(LED_BUILTIN, bOn);
}
#endif

static void s_error_loop(int e1, int e2, int e3, bool loop_forever = true) { // on fatal error, cycle forever, blinking
  int ecode[3] = {e1, e2, e3};
  while (true) {
    s_blink(true, true);
    delay(500);
    s_blink(false, true);
    delay(500);
    for (int ec = 0; ec < 3; ec++) {
      for (int eb = 0; eb < ecode[ec]; eb++) {
        s_blink(true, true);
        delay(100);
        s_blink(false, true);
        delay(100);
      }
      delay(400);
    }
    if (!loop_forever) break;
  }
}

#if defined(ADAFRUIT_FEATHER_M4_CAN)
#  include "FeatherCanbus.hh"
#elif defined(ESP_PLATFORM)
#  include "TwaiCanbus.hh"
#elif defined(ARDUINO_MINIMA)
#  include "R4Canbus.hh"
#elif defined(TEENSYDUINO)
#  include "TeensyCanbus.hh"
#else
#  include "Ada2515Canbus.hh"
#endif

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

class CanbusHandler : public LbCanbus_RequestHandler, public LbCanbus_DataHandler {
private:
  LbCanbus* m_bus;
  uint32_t m_id;
public:
  inline void set_bus(LbCanbus* bus) { m_bus = bus; }

  CanbusHandler(uint32_t id) : m_id(id) {
    // ...
  }
  virtual ~CanbusHandler() {
    // ...
  }
  void canbus_transmission_request(uint32_t packet_id, int length) {
    Serial.print(packet_id, HEX);
    Serial.print(": TR: ");
    Serial.println(length);
  }
  void send_ping() {
    static uint8_t buffer[8] = {'p','i','n','g',':',' ',' ',' '};
    static uint8_t seq_no;

    buffer[6] = s_hex[seq_no >> 4];
    buffer[7] = s_hex[seq_no & 0x0F];
    m_bus->send(m_id, buffer, 8);
    ++seq_no;
  }
  void ping_received(uint32_t packet_id, uint32_t seq_no) {
    static uint8_t buffer[8] = {'p','o','n','g',':',' ',' ',' '};

    buffer[6] = s_hex[seq_no >> 4];
    buffer[7] = s_hex[seq_no & 0x0F];
    m_bus->send(m_id, buffer, 8);
#if 0
    Serial.print(" <<");
#endif
  }
  void pong_received(uint32_t packet_id, uint32_t seq_no) {
    static uint8_t last_seq_no = 0;
    uint8_t expected = last_seq_no + 1;
    if (seq_no != expected) {
      Serial.print(" exp. ");
      Serial.print(expected);
      Serial.print(" rec. ");
      Serial.print(seq_no);
#if 0
    } else {
      Serial.print(" OK");
#else
      Serial.println();
#endif
    }
    last_seq_no = seq_no;
  }
  void canbus_data_received(uint32_t packet_id, const uint8_t* data, int length) {
#if 0
    Serial.print(packet_id, HEX);
    Serial.print(": data(");
    Serial.print(length);
    Serial.print("): ");
    for (int i = 0; i < length; i++) {
      Serial.write(data[i]);
    }
#endif
    if (length == 8) {
      if (memcmp(data, "ping: ", 6) == 0) {
        ping_received(packet_id, s_hex_to_int(data + 6));
      }
      if (memcmp(data, "pong: ", 6) == 0) {
        pong_received(packet_id, s_hex_to_int(data + 6));
      }
    }
#if 0
    Serial.println("");
#endif
  }
};

CanbusHandler handler(27);
LbCanbus* bus = 0;

void setup() {
  Serial.begin(115200);

  s_blink(false);

#if defined(ADAFRUIT_FEATHER_M4_CAN)
  bus = FeatherCanbus::bus();
#elif defined(ESP_PLATFORM)
  bus = TwaiCanbus::bus();
#elif defined(ARDUINO_MINIMA)
  bus = R4Canbus::bus();
#elif defined(TEENSYDUINO)
  bus = Teensy4Canbus::bus();
#else
  bus = Ada2515Canbus::bus();
#endif

  if (bus->begin()) { // defaults to 250kbps
    handler.set_bus(bus);
    bus->set_request_handler(&handler);
    bus->set_data_handler(&handler);
  } else {
    // oops!
    Serial.println("Canbus Error: No available CAN?");
    s_error_loop(1, 1, 1);
  }
}

void loop() {
  static unsigned long u_ref = micros();
  unsigned long u_now = micros();
  static int counter = 0;

  if (u_now < u_ref) {
    u_ref = u_now;
  } else if (u_now - u_ref > 10) { // every 10 microseconds, more or less
    u_ref = u_now;

    bus->spin();

    if (++counter == 20000) { // approx every 200ms
      counter = 0;
      handler.send_ping();
    }
  }

  static int tenth = 0;

  static unsigned long t_ref = millis();
  unsigned long t_now = millis();

  if (t_now < t_ref) {
    t_ref = t_now;
  } else if (t_now - t_ref > 100) { // every 100 milliseconds, more or less
    t_ref = t_now;

    if (++tenth == 10) {
      tenth = 0;
    }
    s_blink((tenth == 0) || (tenth == 2));
  }
}
