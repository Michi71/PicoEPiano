#pragma once
#include <stdint.h>
#include <climits>
#include "pico/stdlib.h"

// Macro to return a value clamped between a minimum and maximum
#ifndef CLAMP
#define CLAMP(a, mn, mx) ((a)<(mx)?((a)>(mn)?(a):(mn)):(mx))
#endif


namespace common {
    static const unsigned int PIN_UNUSED = INT_MAX; // Intentionally INT_MAX to avoid overflowing MicroPython's int type
  
    enum Rotation {
      ROTATE_0 = 0,
      ROTATE_90 = 90,
      ROTATE_180 = 180,
      ROTATE_270 = 270
    };

    enum Polarity {
      ACTIVE_LOW = 0,
      ACTIVE_HIGH = 1
    };

    enum Direction {
      NORMAL_DIR    = 0,
      REVERSED_DIR  = 1,
    };

    inline uint32_t millis() {
      return to_ms_since_boot(get_absolute_time());
    }

  struct pin_pair {
    union {
      uint8_t first;
      uint8_t a;
      uint8_t positive;
      uint8_t phase;
    };
    union {
      uint8_t second;
      uint8_t b;
      uint8_t negative;
      uint8_t enable;
    };

    constexpr pin_pair() : first(0), second(0) {}
    constexpr pin_pair(uint8_t first, uint8_t second) : first(first), second(second) {}
  };

  struct bool_pair {
    union {
      bool first;
      bool a;
    };
    union {
      bool second;
      bool b;
    };

    bool_pair() : first(false), second(false) {}
    bool_pair(bool first, bool second) : first(first), second(second) {}
  };
}
