#include "state.h"

namespace opendoor {

  State::State()
    : unlocked_(false),
      debuggable_(false),
      unlocks_(0),
      door_num_(0x55AA55AA5A5AA5A5)
  {}

  void State::lock() {
    unlocked_ = false;
  }

  void State::unlock() {
    unlocked_ = true;
    unlocks_++;
  }

  bool State::is_unlocked() {
    return unlocked_;
  }

  void State::set_debuggable(bool debuggable) {
    debuggable_ = debuggable;
  }

  bool State::is_debuggable() {
    return debuggable_;
  }

  uint64_t State::get_door_num() {
    return door_num_;
  }

} // namespace opendoor
