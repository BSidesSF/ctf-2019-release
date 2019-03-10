#ifndef _STATE_H_
#define _STATE_H_

#include <stdint.h>

namespace opendoor {

  class State {
    public:
      State();

      void lock();
      void unlock();
      bool is_unlocked();

      void set_debuggable(bool);
      bool is_debuggable();

      uint64_t get_door_num();

    private:
      bool unlocked_;
      bool debuggable_;
      int unlocks_;
      uint64_t door_num_;
  }; // class State

} // namespace opendoor

#endif // _STATE_H_
