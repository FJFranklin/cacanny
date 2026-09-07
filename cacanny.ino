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

class CanbusHandler : public LbCanbus_RequestHandler, public LbCanbus_DataHandler {
public:
  virtual ~CanbusHandler() {
    // ...
  }
  void canbus_transmission_request(uint32_t packet_id, int length) {
    Serial.print(packet_id, HEX);
    Serial.print(": TR: ");
    Serial.println(length);
  }
  void canbus_data_received(uint32_t packet_id, const uint8_t* data, int length) {
    Serial.print(packet_id, HEX);
    Serial.print(": data: ");
    Serial.println(length);
  }
};

CanbusHandler handler;
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

  if (u_now < u_ref) {
    u_ref = u_now;
  } else if (u_now - u_ref > 10) { // every 10 microseconds, more or less
    u_ref = u_now;

    bus->spin();
  }

  static uint8_t buffer[8] = {'p','i','n','g',':',' ',' ',' '};
  static const uint8_t hex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  static uint8_t seq_no = 0;

  static int tenth = 0;

  static unsigned long t_ref = millis();
  unsigned long t_now = millis();

  if (t_now < t_ref) {
    t_ref = t_now;
  } else if (t_now - t_ref > 100) { // every 100 milliseconds, more or less
    t_ref = t_now;

    if (++tenth == 10) {
      tenth = 0;
      buffer[6] = hex[seq_no >> 4];
      buffer[7] = hex[seq_no & 0x0F];
      bus->send(27, buffer, 8); // send full 8 bytes data with ID 27
    }
    s_blink((tenth == 0) || (tenth == 2));
  }
}
