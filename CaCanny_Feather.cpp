/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#if defined(ADAFRUIT_FEATHER_M4_CAN)

#include <CANSAME5x.h>

#include "CaCanny_Feather.hh"

using namespace CaCanny;

static CANSAME5x s_cansame;
static Feather* s_bus = 0;

Feather* Feather::bus(ItemOwner<CanMessage>& store) {
  if (!s_bus) {
    s_bus = new Feather(store, s_cansame);
  }
  return s_bus;
}

Feather::Feather(ItemOwner<CanMessage>& store, CANSAME5x& cansame) :
  Base(store),
  m_CAN(cansame)
{
  pinMode(PIN_CAN_STANDBY, OUTPUT);
  digitalWrite(PIN_CAN_STANDBY, false); // turn off STANDBY
  pinMode(PIN_CAN_BOOSTEN, OUTPUT);
  digitalWrite(PIN_CAN_BOOSTEN, true); // turn on booster
}

bool Feather::begin(Bitrate bitrate) {
  if (bitrate != lb_250kbit) return false; // the feather can handle 500k, but this isn't recommended
  if (!m_CAN.begin(250000)) return false;

  // register the receive callback
  m_CAN.onReceive(receive_callback);

  return true;
}

void Feather::spin() { // check for messages in transmit queue; send only one, if so
  CanMessage* msg = m_transmit.pop();

  if (!msg) { // no messages in transmit queue
    return;
  }

  if (msg->extended) {
    if (msg->remote)
      m_CAN.beginExtendedPacket(msg->id, msg->length, true);
    else
      m_CAN.beginExtendedPacket(msg->id);
  } else {
    if (msg->remote)
      m_CAN.beginPacket(msg->id, msg->length, true);
    else
      m_CAN.beginPacket(msg->id);
  }

  if (!msg->remote) {
    for (int i = 0; i < msg->length; i++) {
      m_CAN.write(msg->buffer[i]);
    }
  }
  m_CAN.endPacket(); // Note: This can hang for up to ~8ms

  msg->return_to_owner(); // return the message instance to the store
}

void Feather::receive_callback(int count) {
  if (s_bus) {
    LinkedList::bISR = true;
    s_bus->receive(count);
    LinkedList::bISR = false;
  }
}

void Feather::receive(int count) {
  ++m_messages_received;

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
}

#endif // ADAFRUIT_FEATHER_M4_CAN
