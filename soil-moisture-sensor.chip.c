#include "wokwi-api.h"
#include <stdint.h>

pin_t out_pin;

void chip_init(void) {
  out_pin = pin_init("OUT", ANALOG);
}

void chip_loop(void) {
  // Nothing required here, Wokwi handles the analog value from slider
}
