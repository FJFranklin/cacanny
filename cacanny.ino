/* -*- mode: c++ -*-
 * 
 * Copyright 2022-26 Francis James Franklin
 * 
 * Open Source under the MIT License - see LICENSE in the project's root folder
 */

#include "CaCanny_Base.hh"

using namespace CaCanny;

class App : public Timer, public Handler {
private:
  volatile ItemOwner<CanMessage>& m_store;
public:
  App(ItemOwner<CanMessage>& store) : m_store(store) {
    // ...
  }
  ~App() {
    // ...
  }

  void every_milli() { // runs once a millisecond, on average
    // ...
  }

  void every_10ms() { // runs once every 10ms, on average
    // ...
  }

  void every_tenth(int tenth) { // runs once every tenth of a second, where tenth = 0..9
    // ...
  }

  void every_second() { // runs once every second
    // ...
  }

  void tick() {
    // ...
  }

  bool will_accept_id(uint32_t id) {
    return true;
  }
};

MessageStore<16> store;

App app(store);

void app_init(Base* base) {
  base->set_handler(&app);
  app.run();
}
