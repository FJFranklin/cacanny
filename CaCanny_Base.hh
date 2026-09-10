/* -*- mode: c++ -*-
 * 
 * Copyright 2026 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#ifndef CaCanny_Base_hh
#define CaCanny_Base_hh

#include "CaCanny_Utils.hh"

namespace CaCanny {

  enum Bitrate {
    lb_250kbit = 0,
    lb_1Mbit
  };

  class Handler {
  public:
    virtual ~Handler() { }
    virtual bool will_accept_id(uint32_t id) = 0;
  };

  class Base {
  private:
    Handler* m_handler;
  protected:
    ItemOwner<CanMessage>& m_store;
    uint32_t m_messages_received;
    uint32_t m_messages_lost; // lost specifically because no CanMessage instances in store
    ItemOwner<CanMessage> m_received;
    ItemOwner<CanMessage> m_transmit;
  public:
    Base(ItemOwner<CanMessage>& store) :
      m_handler(0),
      m_store(store),
      m_messages_received(0),
      m_messages_lost(0)
    {
      // ...
    }
    virtual ~Base() { }

    inline void set_handler(Handler* handler) {
      m_handler = handler;
    }
    inline bool will_accept_id(uint32_t id) {
      if (!m_handler) return true; // by default, accept all
      return m_handler->will_accept_id(id);
    }

    inline void received_message_stats(uint32_t& received, uint32_t& lost) const {
      received = m_messages_received;
      lost = m_messages_lost;
    }

    inline int count() const { // specifically the number of messages in the received queue
      return m_received.count();
    }
    inline CanMessage* next() { // when finished with the message, use message->return_to_owner()
      return m_received.pop();
    }

    inline void send(CanMessage* message) { // control of the message instance passes here
      if (message) m_transmit.push(*message);
    }

    virtual bool begin(Bitrate bitrate = lb_250kbit) = 0;
    virtual void spin() = 0;
  };

} // CaCanny

#endif /* ! CaCanny_Base_hh */
