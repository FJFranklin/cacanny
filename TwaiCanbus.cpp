/* Copyright 2026 Francis James Franklin
 * MIT license: See LICENSE file.
 */

#if defined(ESP_PLATFORM)

#include <driver/twai.h>

#include "TwaiCanbus.hh"

//WIRING PINS
#define CAN_TX_PIN GPIO_NUM_27
#define CAN_RX_PIN GPIO_NUM_26

static TwaiCanbus s_bus;

TwaiCanbus* TwaiCanbus::bus() {
  return &s_bus;
}

TwaiCanbus::TwaiCanbus() {
  // ...
}

TwaiCanbus::~TwaiCanbus() {
  // ...
}

bool TwaiCanbus::begin(LbBitrate bitrate) {
  // Initialize CAN Driver
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);

  if (bitrate == lb_250kbit)
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
  else if (bitrate == lb_1Mbit)
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
  else
    return false;

  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    if (twai_start() == ESP_OK) {
      return true;
    }
  }
  return false;
}

void TwaiCanbus::send(uint32_t packet_id, uint8_t* bytes, int count) {
  // Send standard CAN frame
  twai_message_t message;
  message.identifier = id;
  message.extd = 0;
  message.rtr = 0;  
  message.data_length_code = len;

  for (int i = 0; i < len; i++) {
    message.data[i] = data[i];
  }
  twai_transmit(&message, pdMS_TO_TICKS(10));
}

void TwaiCanbus::receive_callback(int count) {
  // TODO
}
void TwaiCanbus::receive(int count) {
  // TODO
}

void TwaiCanbus::spin() {
  // ...
}

#endif // ESP_PLATFORM
