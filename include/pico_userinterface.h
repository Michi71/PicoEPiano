#ifndef __PICO_USERINTERFACE_H__
#define __PICO_USERINTERFACE_H__

#include <stdio.h>
#include "u8g2.h"
#include "rotary_encoder.h"


// Helper methods which are called from C code
#ifdef __cplusplus
extern "C" {
#endif

uint8_t pico_UserInterfaceSelectionList(u8g2_t *u8g2, RotaryEncoder *enc, const char *title, uint8_t start_pos, const char *sl);
uint8_t pico_UserInterfaceInputValue(u8g2_t *u8g2, RotaryEncoder *enc, const char *title, const char *pre, uint8_t *value, uint8_t lo, uint8_t hi, uint8_t digits, const char *post);


#ifdef __cplusplus
}
#endif

#endif // __PICO_USERINTERFACE_H__
