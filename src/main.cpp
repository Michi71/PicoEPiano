#include <stdio.h>
#include "pico/binary_info.h"
#include "project_config.h"
#include "midi_input_usb.h"
#include "audio_subsystem.h"
#include "pico_hw.h"

#if __has_include("bsp/board_api.h")
#include "bsp/board_api.h"
#else
#include "bsp/board.h"
#endif

#include "u8g2.h"

#include "encoder.h"
#include "push_button.h"
#include "pico_userinterface.h"
//#include "image.h"

#include "FreeRTOS.h"
#include <task.h>
#include <queue.h>

#include "arduino_compat.h"
#include "mdaEPiano.h"

#define USE_DIN_MIDI 0
#define DEBUG_MIDI 0

// Set to 0 if you want to play notes via USB MIDI
#define PLAY_RANDOM_NOTES 1

#ifdef __cplusplus
extern "C"
{
#endif

Encoder enc(pio1, 0, {PIN_ENC_A, PIN_ENC_B});
PushButton bt(PIN_ENC_BTN, 50);

static int8_t ParamChange = 0;

// Create the Audio Buffer
audio_buffer_pool_t *ap;

// Create the Oled screen
u8g2_t u8g2; 

mdaEPiano ep(96);

#define logoWidth 32
#define logoHeight 32

const unsigned char logo [] = //http://javl.github.io/image2cpp/ "Invert image colors" and "Swap bits in byte"
{
	0xfe, 0xff, 0xff, 0xff, 0x06, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0xc0, 
	0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0xbc, 0x03, 0x00, 0xf0, 0x9f, 
	0x03, 0xf0, 0xff, 0x83, 0x83, 0xff, 0xc7, 0x83, 0xe3, 0xc7, 0xc7, 0x83, 0xf3, 0xc7, 0xc7, 0x83, 
	0x9f, 0xc7, 0xc7, 0x83, 0x8f, 0xc7, 0xc7, 0x83, 0x87, 0xc7, 0xc7, 0x83, 0x87, 0xc7, 0xc7, 0x83, 
	0x87, 0xc7, 0xc7, 0x83, 0x83, 0xc7, 0xc7, 0x83, 0x83, 0xc7, 0xc7, 0x83, 0x83, 0xc7, 0xc7, 0x83, 
	0x03, 0x03, 0x81, 0x81, 0x03, 0x03, 0x81, 0x81, 0x03, 0x03, 0x81, 0x81, 0x03, 0x03, 0x81, 0x81, 
	0x03, 0x03, 0x81, 0x81, 0x03, 0x03, 0x81, 0x81, 0x03, 0x03, 0x81, 0x81, 0x03, 0x03, 0x81, 0x81, 
	0x03, 0x03, 0x81, 0x81, 0x06, 0x03, 0x81, 0xc1, 0xfe, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xff, 0x3f
};


MIDIInputUSB usbmidi;
   		   
    // MIDI callbacks
    void note_on_callback(uint8_t note, uint8_t level, uint8_t channel)
    {
        if (level > 0)
        {
            ep.noteOn(note, level);
			gpio_put(PIN_LED, 1);
        }
        else
        {
            ep.noteOff(note);
			gpio_put(PIN_LED, 0);
        }
    }

    void note_off_callback(uint8_t note, uint8_t level, uint8_t channel)
    {
        ep.noteOff(note);
		gpio_put(PIN_LED, 0);
    }

    void cc_callback(uint8_t cc, uint8_t value, uint8_t channel)
    {
        ep.processMidiController(cc, value);
    }

    // This task processes the USB MIDI input
    void usb_midi_task(void *pvParameters)
    {
        usbmidi.setCCCallback(cc_callback);
        usbmidi.setNoteOnCallback(note_on_callback);
        usbmidi.setNoteOffCallback(note_off_callback);

        while (1)
        {
            tud_task();
            usbmidi.process();
        }
    }

    // This tasks generates random notes and plays them.
    // It is only used if PLAY_RANDOM_NOTES is set to 1.
    void play_task(void *pvParameters)
    {
		while (1)
		{
			uint8_t x=rand() % 11;
			uint8_t d=64 + rand() % 63;
	
			note_on_callback(48+x, 90+d, 0);
			vTaskDelay(pdMS_TO_TICKS(100));
			note_on_callback(52+x, 90+d, 0);
			vTaskDelay(pdMS_TO_TICKS(100));
			note_on_callback(55+x, 90+d, 0);
			vTaskDelay(pdMS_TO_TICKS(100));
			note_on_callback(60+x, 90+d, 0);
			vTaskDelay(pdMS_TO_TICKS(2000));

			note_off_callback(48+x, 0, 0);
			note_off_callback(52+x, 0, 0);
			note_off_callback(55+x, 0, 0);
			note_off_callback(60+x, 0, 0);
			vTaskDelay(pdMS_TO_TICKS(2000));
		}
    }
		
	// GUI
    void gui_task(void *p)
    {	
		int8_t res;
				
        u8g2_ClearDisplay(&u8g2);
		u8g2_SetDrawColor(&u8g2, 1);		
		
		while (1)
		{
			// Select Program
			res = pico_UserInterfaceProgramSelect(&u8g2, &enc, &bt, &ep);
			
			while (res > -1) {
				vTaskDelay(pdMS_TO_TICKS(500));
				res = pico_UserInterfaceParamSelect(&u8g2, &enc, &bt, &ep);
				if (res > -1)
				{
					vTaskDelay(pdMS_TO_TICKS(500));
					pico_UserInterfaceParamInput(&u8g2, &enc, &bt, &ep, res-1);
				//	ep.flashWriteProgram();
				}
			}
			
			vTaskDelay(pdMS_TO_TICKS(500));
		}
	}

    int main(void)
    {
	
        // initialize the hardware
        pico_init();	// Board	
		bt.Init();		// Button
		enc.init();		// Encoder
		
		// Initialize the audio subsystem
        ap = init_audio();
		ep.setVolume(64); 

		// Initialize the OLED screen (SH1106)
		u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, 
										  U8G2_R0, 
										  u8x8_byte_pico_hw_i2c, 
										  u8x8_gpio_and_delay_pico);	 
		u8g2_InitDisplay(&u8g2);
		u8g2_SetPowerSave(&u8g2, 0); 	
        u8g2_ClearBuffer(&u8g2);
		
		// Show Splash Screen
		u8g2_SetFont(&u8g2, u8g2_font_8x13B_tf);
		u8g2_SetBitmapMode(&u8g2, false);		
		u8g2_FirstPage(&u8g2);
		do 
		{
			u8g2_DrawXBMP(&u8g2, 2, 15, logoWidth, logoHeight, logo);
			u8g2_DrawUTF8(&u8g2, 40, 25, PICO_PROGRAM_NAME);			
			u8g2_DrawUTF8(&u8g2, 40, 40, PICO_PROGRAM_VERSION_STRING);
		} while (u8g2_NextPage(&u8g2));
		
		sleep_ms(2000);
										  
		/*while (1) {		
			gpio_put(25, 1);
			sleep_ms(500);
			gpio_put(25, 0);
			sleep_ms(500);
		}*/

		// Create FreeRTOS Tasks for USB MIDI and printing statistics
        xTaskCreate(usb_midi_task, "USBMIDI", 4096, NULL, configMAX_PRIORITIES, NULL);
		xTaskCreate(gui_task, "GUI", 4096, NULL, configMAX_PRIORITIES - 1, NULL);
#if PLAY_RANDOM_NOTES
        xTaskCreate(play_task, "PLAY", 1024, NULL, configMAX_PRIORITIES - 1, NULL);
#endif

        // Start the scheduler.
        vTaskStartScheduler();

        // Idle loop.
        while (1)
        {
            ;
            ;
        } 
    }

    // This fis the I2S callback function. It is called when the I2S subsystem
    // needs more audio data. It is called at a fixed rate of 48kHz.
    // The audio data is stored in the audio_buffer_t struct.
    void __not_in_flash_func(i2s_callback_func())
    {
		if (ParamChange != 0) return;
		
        audio_buffer_t *buffer = take_audio_buffer(ap, false);
        if (buffer == NULL)
        {
            return;
        }
		int16_t l[buffer->max_sample_count];
		int16_t r[buffer->max_sample_count];
		
        int32_t *samples = (int32_t *)buffer->buffer->bytes;

		// Process the synth
		ep.process(&l[0], &r[0]);

        // We are filling the buffer with 32-bit samples (2 channels)
        for (uint i = 0; i < buffer->max_sample_count; i++)
        {
			samples[i * 2 + 0] = l[i] << 16;  	// LEFT
            samples[i * 2 + 1] = r[i] << 16; 	// RIGHT
        }

        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(ap, buffer);
        return;
    }

#ifdef __cplusplus
}
#endif
