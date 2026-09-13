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
      return true;
    }
  }

  // FIXME: add callback for receive

  return false;
}

void TWAI::spin() { // check for messages in transmit queue; send only one, if so
  CanMessage* msg = m_transmit.pop();

  if (!msg) { // no messages in transmit queue
    return;
  }

  twai_message_t packet;
  packet.identifier = msg->id;
  packet.extd = msg->extended;
  packet.rtr = msg->remote;
  packet.data_length_code = msg->length;

  if (!msg->remote) {
    for (int i = 0; i < msg->length; i++) {
      packet.data[i] = msg->buffer[i];
    }
  }
  twai_transmit(&packet, pdMS_TO_TICKS(10));

  msg->return_to_owner(); // return the message instance to the store
}

void TWAI::receive_callback(int count) {
  if (s_bus) {
    LinkedList::bISR = true;
    s_bus->receive(count);
    LinkedList::bISR = false;
  }
}

void TWAI::receive(int count) {
  ++m_messages_received;
  /* FIXME
  CanMessage* msg = 0;

  if (will_accept_id(m_CAN.packetId())) { // is the packet ID relevant to us?
    msg = m_store.pop();
    if (!msg) { // no messages in store - oops
      ++m_messages_lost;
    }
  }

  if (m_CAN.packetRtr()) { // Remote transmission request, packet contains no data
    if (msg) {
      msg->transmission_request(m_CAN.packetId(), m_CAN.packetDlc());
      msg->extended = m_CAN.packetExtended();
    }
  } else {
    if (msg) {
      msg->data_frame(m_CAN.packetId(), (count > 8) ? 8 : count);
      msg->extended = m_CAN.packetExtended();
    }
    for (int i = 0; i < count; i++) {
      uint8_t c = m_CAN.read();
      if (msg && (i < 8))
        msg->buffer[i] = c;
    }
  }
  if (msg) {
    m_received.push(*msg);
  }
  */
}

#endif // ESP_PLATFORM
