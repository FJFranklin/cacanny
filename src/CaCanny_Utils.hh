/* -*- mode: c++ -*-
 * 
 * Copyright 2022-26 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#ifndef __CaCanny_Utils_hh__
#define __CaCanny_Utils_hh__

#include <Arduino.h>

namespace CaCanny {

  class LinkedItem;
  class LinkedList;

  class LinkedItemOwner {
    friend LinkedItem;
  protected:
    virtual void linked_item_push(LinkedItem& item) = 0;
    virtual LinkedItem* linked_item_pop() = 0;
  public:
    virtual ~LinkedItemOwner() { }
  };

  class LinkedItem {
    friend LinkedList;
  private:
    LinkedItem* m_next;
    LinkedItemOwner* m_owner;
  public:
    LinkedItem() :
      m_next(0),
      m_owner(0)
    {
      // ...
    }
    virtual ~LinkedItem() {
      // ...
    }

    inline void return_to_owner() {
      if (m_owner)
        m_owner->linked_item_push(*this);
    }
  };

  class LinkedList : public LinkedItemOwner {
  private:
    LinkedItem* m_next;
    int m_count;
  public:
    static bool bISR;

    LinkedList() : m_next(0), m_count(0) {
      // ...
    }
    virtual ~LinkedList() { }
  protected:
    inline void linked_item_adopt(LinkedItem& item) {
      item.m_owner = this;
    }
    virtual void linked_item_push(LinkedItem& item);
    virtual LinkedItem* linked_item_pop();
    LinkedItem* linked_item(int index) const;
  public:
    inline int count() const {
      return m_count;
    }
    inline void pop_and_return() {
      LinkedItem* iptr = linked_item_pop();
      if (iptr)
        iptr->return_to_owner();
    }
  };

  template<class T> class ItemOwner : public LinkedList {
  public:
    virtual ~ItemOwner() { }

    inline void push(T& item, bool bAdopt = false) {
      if (bAdopt)
        linked_item_adopt(item);
      linked_item_push(item);
    }
    inline T* pop() {
      return (T*) linked_item_pop();
    }
    inline const T* item(int index) const {
      return (const T*) linked_item(index);
    }
  };

  class CanMessage : public LinkedItem {
  public:
    uint8_t buffer[8];
    uint8_t length;

    uint32_t id;

    bool bExtended;
    bool bTransReq;

    CanMessage() :
      length(0),
      id(0),
      bExtended(false),
      bTransReq(false)
    {
      // ...
    }
    virtual ~CanMessage() {
      // ...
    }
  };

  template<uint8_t MSCount>
  class MessageStore : public ItemOwner<CanMessage> {
  private:
    CanMessage m_allocation[MSCount];
  public:
    MessageStore() {
      for (uint8_t i = 0; i < MSCount; i++) {
	push(m_allocation[i], true);
      }
    }
    virtual ~MessageStore() {
      // ...
    }
  };

  class Timer {
  private:
    bool m_stop;

  public:
    Timer() :
      m_stop(false)
    {
      // ...
    }

    virtual ~Timer();

    virtual void every_milli();          // runs once a millisecond, on average
    virtual void every_10ms();           // runs once every 10ms, on average
    virtual void every_tenth(int tenth); // runs once every tenth of a second, where tenth = 0..9
    virtual void every_second();         // runs once every second
    virtual void tick();

    inline void stop() {
      m_stop = true;
    }
    void run();
  };

} // CaCanny

#endif /* !__CaCanny_Utils_hh__ */
