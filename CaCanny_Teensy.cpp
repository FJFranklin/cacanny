/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#if defined(TEENSYDUINO)

#include <FlexCAN_T4.h>

#include "CaCanny_Teensy.hh"

using namespace CaCanny;

static FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16> s_can1;
static FlexCAN_T4<CAN2, RX_SIZE_16, TX_SIZE_16> s_can2;
static FlexCAN_T4<CAN3, RX_SIZE_16, TX_SIZE_16> s_can3;

static Teensy* s_bus1 = 0;
static Teensy* s_bus2 = 0;
static Teensy* s_bus3 = 0;

Teensy* Teensy::bus(ItemOwner<CanMessage>& store, TeensyBus can_no) {
  Teensy* instance = 0;

  switch (can_no) {
  case TeensyBus::tc_bn_1:
    if (!s_bus1) s_bus1 = new Teensy(store, "Teensy Can 1", &s_can1, TeensyBus::tc_bn_1);
    instance = s_bus1;
    break;
  case TeensyBus::tc_bn_2:
    if (!s_bus2) s_bus2 = new Teensy(store, "Teensy Can 2", &s_can2, TeensyBus::tc_bn_2);
    instance = s_bus2;
    break;
  case TeensyBus::tc_bn_3:
    if (!s_bus3) s_bus3 = new Teensy(store, "Teensy Can 3", &s_can3, TeensyBus::tc_bn_3);
    instance = s_bus3;
    break;
  }
  return instance;
}

void Teensy::bus1_sniff(const CAN_message_t& msg) {
  if (s_bus1) {
    LinkedList::bISR = true;
    s_bus1->receive(msg);
    LinkedList::bISR = false;
  }
}

void Teensy::bus2_sniff(const CAN_message_t& msg) {
  if (s_bus2) {
    LinkedList::bISR = true;
    s_bus2->receive(msg);
    LinkedList::bISR = false;
  }
}

void Teensy::bus3_sniff(const CAN_message_t& msg) {
  if (s_bus3) {
    LinkedList::bISR = true;
    s_bus3->receive(msg);
    LinkedList::bISR = false;
  }
}

Teensy::Teensy(ItemOwner<CanMessage>& store, const char* const backend, FlexCAN_T4_Base* can_base, TeensyBus can_no) :
  Base(store, backend),
  m_can_no(can_no),
  m_can_base(can_base)
{
  // pinMode(6, OUTPUT); digitalWrite(6, LOW); /* optional tranceiver enable pin */
}

bool Teensy::begin(Bitrate bitrate) {
  unsigned long CAN_BAUDRATE = 250000;

  if (bitrate != lb_250kbit) {
    if (bitrate == lb_1Mbit)
      CAN_BAUDRATE = 1000000;
    else
      return false;
  }

  switch (m_can_no) {
  case TeensyBus::tc_bn_1:
    s_can1.begin();
    s_can1.setBaudRate(CAN_BAUDRATE);
    s_can1.setMaxMB(16);
    s_can1.enableFIFO();
    s_can1.enableFIFOInterrupt();
    s_can1.onReceive(bus1_sniff);
    break;
  case TeensyBus::tc_bn_2:
    s_can2.begin();
    s_can2.setBaudRate(CAN_BAUDRATE);
    s_can2.setMaxMB(16);
    s_can2.enableFIFO();
    s_can2.enableFIFOInterrupt();
    s_can2.onReceive(bus2_sniff);
    break;
  case TeensyBus::tc_bn_3:
    s_can3.begin();
    s_can3.setBaudRate(CAN_BAUDRATE);
    s_can3.setMaxMB(16);
    s_can3.enableFIFO();
    s_can3.enableFIFOInterrupt();
    s_can3.onReceive(bus3_sniff);
    break;
  }
  return true;
}

void Teensy::spin() { // check for messages in transmit queue; send only one, if so
  m_can_base->events();

  CanMessage* msg = m_transmit.pop();

  if (!msg) { // no messages in transmit queue
    return;
  }

  CAN_message_t packet;

  packet.len = msg->length;
  packet.id = msg->id;
  packet.flags.extended = msg->extended;
  memcpy(packet.buf, msg->buffer, msg->length);
  
  m_can_base->write(packet);

  msg->return_to_owner(); // return the message instance to the store
}

void Teensy::receive(const CAN_message_t& packet) {
  ++m_messages_received;

  CanMessage* msg = 0;

  if (will_accept_id(packet.id)) { // is the packet ID relevant to us?
    msg = m_store.pop();
    if (!msg) { // no messages in store - oops
      ++m_messages_lost;
    }
  }
  if (!msg) return;

  if (packet.flags.remote) { // Remote transmission request, packet contains no data
    msg->transmission_request(packet.id, packet.len); // FIXME - Check msg.len is requested transmission length
  } else {
    msg->data_frame(packet.id, packet.len);
  }
  memcpy(msg->buffer, packet.buf, packet.len);

  m_received.push(*msg);
}

#endif // TEENSYDUINO
