/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#include "CaCanny_config.hh"

#if ENABLE_MCP2515

#include <Adafruit_MCP2515.h>

#include "CaCanny_MCP2515.hh"

using namespace CaCanny;

static MCP2515* s_bus = 0;

MCP2515* MCP2515::bus(ItemOwner<CanMessage>& store, int pin_chip_select, int pin_interrupt) {
  if (!s_bus) {
    pinMode(pin_chip_select, OUTPUT);
    pinMode(pin_interrupt, INPUT);
    s_bus = new MCP2515(store, pin_chip_select, pin_interrupt);
  }
  return s_bus;
}

MCP2515::MCP2515(ItemOwner<CanMessage>& store, int pin_chip_select, int pin_interrupt) :
  Base(store),
  m_mcp(new Adafruit_MCP2515(pin_chip_select)),
  m_pin_interrupt(pin_interrupt)
{
  // ...
}

MCP2515::~MCP2515() {
  delete m_mcp;
}

bool MCP2515::begin(Bitrate bitrate) {
  unsigned long CAN_BAUDRATE = 250000;

  if (bitrate != lb_250kbit) {
    if (bitrate == lb_1Mbit)
      CAN_BAUDRATE = 1000000;
    else
      return false;
  }
  if (!m_mcp->begin(CAN_BAUDRATE)) {
    return false;
  }

  // register the receive callback
  m_mcp->onReceive(m_pin_interrupt, receive_callback);

  return true;
}

void MCP2515::spin() { // check for messages in transmit queue; send only one, if so
  CanMessage* msg = m_transmit.pop();

  if (!msg) { // no messages in transmit queue
    return;
  }

  if (msg->extended) {
    if (msg->remote)
      m_mcp->beginExtendedPacket(msg->id, msg->length, true);
    else
      m_mcp->beginExtendedPacket(msg->id);
  } else {
    if (msg->remote)
      m_mcp->beginPacket(msg->id, msg->length, true);
    else
      m_mcp->beginPacket(msg->id);
  }

  if (!msg->remote) {
    for (int i = 0; i < msg->length; i++) {
      m_mcp->write(msg->buffer[i]);
    }
  }
  m_mcp->endPacket();

  msg->return_to_owner(); // return the message instance to the store
}

void MCP2515::receive_callback(int count) {
  if (s_bus) {
    LinkedList::bISR = true;
    s_bus->receive(count);
    LinkedList::bISR = false;
  }
}

void MCP2515::receive(int count) {
  ++m_messages_received;

  CanMessage* msg = 0;

  uint32_t packet_id = m_mcp->packetId();

  if (will_accept_id(packet_id)) { // is the packet ID relevant to us?
    msg = m_store.pop();
    if (!msg) { // no messages in store - oops
      ++m_messages_lost;
    }
  }

  if (m_mcp->packetRtr()) { // Remote transmission request, packet contains no data
    if (msg) {
      msg->transmission_request(packet_id, m_mcp->packetDlc());
      msg->extended = m_mcp->packetExtended();
    }
  } else {
    if (msg) {
      msg->data_frame(packet_id, (count > 8) ? 8 : count);
      msg->extended = m_mcp->packetExtended();
    }
    for (int i = 0; i < count; i++) {
      uint8_t c = m_mcp->read();
      if (msg && (i < 8))
        msg->buffer[i] = c;
    }
  }
  if (msg) {
    m_received.push(*msg);
  }
}

#endif // ENABLE_MCP2515
