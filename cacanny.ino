// -*-c++-*-

#if defined(ADAFRUIT_FEATHER_M4_CAN)
#  include "FeatherCanbus.hh"
#elif defined(ESP_PLATFORM)
#  include "TwaiCanbus.hh"
#elif defined(ARDUINO_MINIMA)
#  include "R4Canbus.hh"
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

#if defined(ADAFRUIT_FEATHER_M4_CAN)
  bus = FeatherCanbus::bus();
#elif defined(ESP_PLATFORM)
  bus = TwaiCanbus::bus();
#elif defined(ARDUINO_MINIMA)
  bus = R4Canbus::bus();
#else
  bus = Ada2515Canbus::bus();
#endif

  if (bus->begin()) { // defaults to 250kbps
    bus->set_request_handler(&handler);
    bus->set_data_handler(&handler);
  } else {
    // oops!
    Serial.println("Feather Canbus Error");
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

  static int tenth = 0;

  static unsigned long t_ref = millis();
  unsigned long t_now = millis();

  if (t_now < t_ref) {
    t_ref = t_now;
  } else if (t_now - t_ref > 100) { // every 100 milliseconds, more or less
    t_ref = t_now;

    if (++tenth == 10) tenth = 0;

    digitalWrite(LED_BUILTIN, (tenth == 0) || (tenth == 2)); // FIXME: Feather uses the neopixel?
  }
}
