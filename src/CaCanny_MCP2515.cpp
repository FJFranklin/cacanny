/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#include "CaCanny_config.hh"

#if ENABLE_MCP2515_Adafruit
#include <Adafruit_MCP2515.h>
#endif
#if ENABLE_MCP2515_autowp
#include <mcp2515.h>
#endif

#include "CaCanny_MCP2515.hh"

using namespace CaCanny;

static CC_MCP2515* s_bus = 0;

CC_MCP2515* CC_MCP2515::bus(ItemOwner<CanMessage>& store, int pin_chip_select, int pin_interrupt) {
  if (!s_bus) {
    s_bus = new CC_MCP2515(store, pin_chip_select, pin_interrupt);
  }
  return s_bus;
}

CC_MCP2515::CC_MCP2515(ItemOwner<CanMessage>& store, int pin_chip_select, int pin_interrupt) :
  Base(store, "MCP2515"),
#if ENABLE_MCP2515_Adafruit
  m_mcp(new Adafruit_MCP2515(pin_chip_select)),
#endif
#if ENABLE_MCP2515_autowp
  m_mcp(new MCP2515(pin_chip_select)),
  // MCP2515(const uint8_t _CS, const uint32_t _SPI_CLOCK = DEFAULT_SPI_CLOCK, SPIClass * _SPI = nullptr)
  // const uint32_t DEFAULT_SPI_CLOCK = 10000000; // 10MHz
  m_msg(new can_frame),
#endif
  m_pin_chip_select(pin_chip_select),
  m_pin_interrupt(pin_interrupt)
{
  // ...
}

CC_MCP2515::~CC_MCP2515() {
#if ENABLE_MCP2515_Adafruit
  delete m_mcp;
#endif
#if ENABLE_MCP2515_autowp
  delete m_mcp;
  delete m_msg;
#endif
}

bool CC_MCP2515::begin(Bitrate bitrate) {
  bool bSetupComplete = false;
#if ENABLE_MCP2515_Adafruit
  unsigned long CAN_BAUDRATE = 250000;

  if (bitrate != lb_250kbit) {
    if (bitrate == lb_1Mbit)
      CAN_BAUDRATE = 1000000;
    else
      return false;
  }

  pinMode(m_pin_chip_select, OUTPUT);
  pinMode(m_pin_interrupt, INPUT);

  if (!m_mcp->begin(CAN_BAUDRATE)) {
    return false;
  } else {
    bSetupComplete = true;
  }

  // register the receive callback
  m_mcp->onReceive(m_pin_interrupt, receive_callback);
#endif
#if ENABLE_MCP2515_autowp
  CAN_SPEED CAN_BAUDRATE = CAN_250KBPS;

  if (bitrate != lb_250kbit) {
    if (bitrate == lb_1Mbit)
      CAN_BAUDRATE = CAN_1000KBPS;
    else
      return false;
  }

  if (m_mcp->reset() == MCP2515::ERROR_OK) {
    m_mcp->setBitrate(CAN_BAUDRATE);
    m_mcp->setNormalMode();

    bSetupComplete = true;
  }
#endif
  return bSetupComplete;
}

void CC_MCP2515::spin() { // check for messages in transmit queue; send only one, if so
  CanMessage* msg = m_transmit.pop();

#if ENABLE_MCP2515_Adafruit
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
#endif
#if ENABLE_MCP2515_autowp
  /* Check for incoming
   */
  if (m_mcp->readMessage(m_msg) == MCP2515::ERROR_OK) {
    if (m_msg->can_dlc <= 8 && !(m_msg->can_id & CAN_ERR_FLAG))
      receive(m_msg->can_dlc);
  }

  /* Check for outgoing
   */
  if (!msg) { // no messages in transmit queue
    return;
  }

  m_msg->can_id = msg->id;
  if (msg->extended)
    m_msg->can_id |= CAN_EFF_FLAG;

  m_msg->can_dlc = msg->length;

  if (msg->remote) {
    m_msg->can_id |= CAN_RTR_FLAG;
  } else {
    for (int i = 0; i < msg->length; i++) {
      m_msg->data[i] = msg->buffer[i];
    }
  }
  m_mcp->sendMessage(m_msg);
#endif
  msg->return_to_owner(); // return the message instance to the store
}

void CC_MCP2515::receive_callback(int count) {
  if (s_bus) {
    LinkedList::bISR = true;
    s_bus->receive(count);
    LinkedList::bISR = false;
  }
}

void CC_MCP2515::receive(int count) {
  ++m_messages_received;

  CanMessage* msg = 0;
#if ENABLE_MCP2515_Adafruit
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
#endif
#if ENABLE_MCP2515_autowp
  uint32_t packet_id = m_msg->can_id & CAN_EFF_MASK;

  if (will_accept_id(packet_id)) { // is the packet ID relevant to us?
    msg = m_store.pop();
    if (!msg) { // no messages in store - oops
      ++m_messages_lost;
    }
  }

  if (m_msg->can_id & CAN_RTR_FLAG) { // Remote transmission request, packet contains no data
    if (msg) {
      msg->transmission_request(packet_id, count);
      msg->extended = m_msg->can_id & CAN_EFF_FLAG;
    }
  } else {
    if (msg) {
      msg->data_frame(packet_id, count);
      msg->extended = m_msg->can_id & CAN_EFF_FLAG;
    }
    for (int i = 0; i < count; i++) {
      msg->buffer[i] = m_msg->data[i];
    }
  }
#endif
  if (msg) {
    m_received.push(*msg);
  }
}
