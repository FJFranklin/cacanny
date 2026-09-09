/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#if defined(ARDUINO_MINIMA)

#include <Arduino_CAN.h>

#include "CaCanny_Minima.hh"

using namespace CaCanny;

static Minima* s_bus = 0;

Minima* Minima::bus(ItemOwner<CanMessage>& store) {
  if (!s_bus) {
    s_bus = new Minima(store);
  }
  return s_bus;
}

Minima::Minima(ItemOwner<CanMessage>& store) :
  Base(store)
{
  // ...
}

bool Minima::begin(Bitrate bitrate) {
  CanBitRate CAN_BAUDRATE = CanBitRate::BR_250k; // see ArduinoCore-API/api/HardwareCAN.h

  if (bitrate != lb_250kbit) {
    if (bitrate == lb_1Mbit)
      CAN_BAUDRATE = CanBitRate::BR_1000k;
    else
      return false;
  }
  if (!CAN.begin(CAN_BAUDRATE)) {
    return false;
  }

  return true;
}

void Minima::spin() {
  /* check for incoming messages and add to receive queue
   */
  while (CAN.available()) {
    // RTR reserved but not implemented

    CanMsg const packet = CAN.read();

    uint32_t packet_id = packet.getExtendedId();

    ++m_messages_received;

    CanMessage* msg = 0;

    if (will_accept_id(packet_id)) { // is the packet ID relevant to us?
      msg = m_store.pop();
      if (!msg) { // no messages in store - oops
        ++m_messages_lost;
      }
    }
    if (msg) {
      msg->data_frame(packet_id, packet.data_length);
      memcpy(msg->buffer, packet.data, packet.data_length);
      m_received.push(*msg);
    }
  }

  /* check for messages in transmit queue; send only one, if so
   */
  CanMessage* msg = m_transmit.pop();

  if (msg) { // there's a message in transmit queue
    if (!msg->remote) { // Arduino R4 doesn't currently support transmission requests
      CAN.write(CanMsg(msg->id, msg->length, msg->buffer));
    }
    msg->return_to_owner(); // return the message instance to the store
  }
}

#endif // ARDUINO_MINIMA
