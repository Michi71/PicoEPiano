#ifndef _rotary_encoder_h
#define _rotary_encoder_h

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"


class RotaryEncoder
{
	public:
		RotaryEncoder(PIO pio, uint sm, uint rotary_encoder_A);
		
		int get_rotation(void);
		int get_delta(void);
		
		int get_button(void);
								
	private:
		// the pio instance
		PIO _pio;
		// the state machine
		uint _sm;	
		// the state machine for the button
		uint _smbt;
		// the offset of the button debouncer program
		uint _offset;		
		
		// the current location of rotation
		static int rotation;	
		static int old_rotation;			
		static int delta;		
};


#endif
