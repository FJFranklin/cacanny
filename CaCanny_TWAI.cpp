/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#if defined(ESP_PLATFORM)

#include <driver/twai.h>

#include "CaCanny_TWAI.hh"

using namespace CaCanny;

static TWAI* s_bus = 0;

TWAI* TWAI::bus(ItemOwner<CanMessage>& store, int pin_tx, int pin_rx) {
  if (!s_bus) {
    s_bus = new TWAI(store, pin_tx, pin_rx);
  }
  return s_bus;
}

TWAI::TWAI(ItemOwner<CanMessage>& store, int pin_tx, int pin_rx) :
  Base(store, "TWAI"),
  m_pin_tx(pin_tx),
  m_pin_rx(pin_rx)
{
  // ...
}

bool TWAI::begin(Bitrate bitrate) {
  // Initialize CAN Driver
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t) m_pin_tx, (gpio_num_t) m_pin_rx, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config;

  if (bitrate == lb_250kbit)
    t_config = TWAI_TIMING_CONFIG_250KBITS();
  else if (bitrate == lb_1Mbit)
    t_config = TWAI_TIMING_CONFIG_1MBITS();
  else
    return false;

  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    if (twai_start() == ESP_OK) {
    //if (twai_reconfigure_alerts(TWAI_ALERT_RX_DATA, 0) == ESP_OK) {
        return true;
    //}
    }
  }

  return false;
}

void TWAI::spin() { // check for messages in transmit queue; send only one, if so
  twai_status_info_t twaistatus;
  twai_get_status_info(&twaistatus);
  for (uint32_t p = 0; p < twaistatus.msgs_to_rx; p++) {
    ++m_messages_received;

    twai_message_t packet;
    if (twai_receive(&packet, 0) != ESP_OK) { // oops ??
      ++m_messages_lost;
      break;
    }
    if (!will_accept_id(packet.identifier)) continue; // not interested in this one

    CanMessage* msg = m_store.pop();
    if (!msg) { // no message instances in store
      ++m_messages_lost;
      break;
    }
    if (packet.rtr) { // Remote transmission request, packet contains no data
      msg->transmission_request(packet.identifier, packet.data_length_code);
      msg->extended = packet.extd;
    } else {
      msg->data_frame(packet.identifier, (packet.data_length_code > 8) ? 8 : packet.data_length_code);
      msg->extended = packet.extd;
      memcpy(msg->buffer, packet.data, msg->length);
    }
    m_received.push(*msg);
  }
  
  CanMessage* msg = m_transmit.pop();

  if (!msg) { // no messages in transmit queue
    return;
  }

  twai_message_t packet;
  packet.identifier = msg->id;
  packet.extd = msg->extended;
  packet.rtr = msg->remote;
  packet.data_length_code = msg->length;

  if (!msg->remote) { // FIXME
    for (int i = 0; i < msg->length; i++) {
      packet.data[i] = msg->buffer[i];
    }
  }
  twai_transmit(&packet, pdMS_TO_TICKS(10));

  msg->return_to_owner(); // return the message instance to the store
}

#endif // ESP_PLATFORM
