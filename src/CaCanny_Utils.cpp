/* -*- mode: c++ -*-
 * 
 * Copyright 2022-26 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#if defined(ADAFRUIT_FEATHER_M4_CAN)
#include <Adafruit_NeoPixel.h>
#endif
#if defined(ARDUINO_UNOR4_WIFI)
#include <Arduino_LED_Matrix.h>
#endif

#include "CaCanny_Utils.hh"

using namespace CaCanny;

volatile bool LinkedList::bISR = false;

void LinkedList::linked_item_push(LinkedItem& item) {
  if (!bISR) noInterrupts();

  if (!m_next) {
    m_next = &item;
  } else {
    LinkedItem* iptr = m_next;
    while (iptr->m_next)
      iptr = iptr->m_next;
    iptr->m_next = &item;
  }
  item.m_next = 0;
  ++m_count;

  if (!bISR) interrupts();
}

LinkedItem* LinkedList::linked_item_pop() {
  if (!bISR) noInterrupts();

  LinkedItem* iptr = m_next;
  if (iptr) {
    m_next = iptr->m_next;
    iptr->m_next = 0;
    --m_count;
  }

  if (!bISR) interrupts();
  return iptr;
}

LinkedItem* LinkedList::linked_item(int index) const { // is this used?
  if (!bISR) noInterrupts();

  LinkedItem* iptr = m_next;

  if (index >= 0 && index < m_count) {
    while (index--)
      iptr = iptr->m_next;
  }

  if (!bISR) interrupts();
  return iptr;
}

Timer::~Timer() {
  // ...
}

void Timer::every_milli() { // runs once a millisecond, on average
  // ...
}

void Timer::every_10ms() { // runs once every 10ms, on average
  // ...
}

void Timer::every_tenth(int tenth) { // runs once every tenth of a second, where tenth = 0..9
  // ...
}

void Timer::every_second() { // runs once every second
  // ...
}

void Timer::tick() {
  // ...
}

void Timer::run() {
  int count_ms = 0;
  int count_10ms = 0;
  int count_tenths = 0;

  unsigned long previous_time = millis();

  m_stop = false;

  while (!m_stop) {
    tick();

    // our little internal real-time clock:
    unsigned long current_time = millis();

    if (current_time != previous_time) {
      ++previous_time;
      every_milli();

      if (++count_ms == 10) {
	count_ms = 0;
	every_10ms();

	if (++count_10ms == 10) {
	  count_10ms = 0;
	  every_tenth(count_tenths);

	  if (++count_tenths == 10) {
	    count_tenths = 0;
	    every_second();
	  }
	}
      }
    }
    yield(); // just in case it's needed, e.g., to reset the watchdog timer on the ESP8266
  }
}

#if defined(ADAFRUIT_FEATHER_M4_CAN)
static Adafruit_NeoPixel s_pixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
static LED s_led(&s_pixel);
#elif defined(ARDUINO_UNOR4_WIFI)
ArduinoLEDMatrix s_matrix;
static LED s_led(&s_matrix);
#else
static LED s_led;
#endif

LED* LED::onboard_LED() {
  return &s_led;
}

LED::LED(Adafruit_NeoPixel* pixel) : m_pixel(pixel), m_matrix(0), m_state(false), m_error(false) {
#if defined(ADAFRUIT_FEATHER_M4_CAN)
  if (m_pixel) {
    m_pixel->begin();
    m_pixel->setPixelColor(0, m_pixel->Color(0, 255, 0));
    m_pixel->setBrightness(1);
    m_pixel->show();
  }
#else
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
#endif
}

LED::LED(ArduinoLEDMatrix* matrix) : m_pixel(0), m_matrix(matrix), m_state(false), m_error(false) {
#if defined(ARDUINO_UNOR4_WIFI)
  const unsigned long frame[] = {
      0x70000000,
      0x0,
      0x0
  };
  matrix->loadFrame(frame);
  matrix->begin();
#endif
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
}

void LED::blink(bool bOn) {
  if (m_state == bOn) return; // don't waste time changing the state if it's already correct
#if defined(ADAFRUIT_FEATHER_M4_CAN)
  if (m_pixel) {
    m_pixel->setBrightness(bOn ? 31 : 1);
    m_pixel->show();
  }
#elif defined(RGB_BUILTIN)
  if (bOn) {
    if (m_error)
      rgbLedWrite(RGB_BUILTIN, 64, 0, 0);
    else
      rgbLedWrite(RGB_BUILTIN, 0, 64, 0);
  } else {
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
  }
#else
  digitalWrite(LED_BUILTIN, bOn);
#endif
  m_state = bOn;
}

void LED::error(int e1, int e2, int e3, bool loop_forever) { // on fatal error, cycle forever, blinking
#if defined(ADAFRUIT_FEATHER_M4_CAN)
  if (m_pixel) {
    m_pixel->setPixelColor(0, m_pixel->Color(255, 0, 0)); // switch pixel to red
  }
#endif
  m_error = true;
  
  int ecode[3] = {e1, e2, e3};
  while (true) {
    blink(true);
    delay(500);
    blink(false);
    delay(500);
    for (int ec = 0; ec < 3; ec++) {
      for (int eb = 0; eb < ecode[ec]; eb++) {
        blink(true);
        delay(100);
        blink(false);
        delay(100);
      }
      delay(400);
    }
    if (!loop_forever) break;
  }

#if defined(ADAFRUIT_FEATHER_M4_CAN)
  if (m_pixel) {
    m_pixel->setPixelColor(0, m_pixel->Color(0, 255, 0)); // switch pixel to green
  }
#endif
  m_error = false;
}
