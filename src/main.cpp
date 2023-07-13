#include <stdio.h>
#include "project_config.h"
#include "bsp/board.h"
#include "midi_input_usb.h"
#include "audio_subsystem.h"
#include "pico_hw.h"

#include "u8g2.h"

#include "rotary_encoder.h"
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

RotaryEncoder enc(pio1, 0, PIN_ENC_A);

static int8_t ParamChange = 0;

// Create an encoder on the 3 ADC pins, using PIO 0 and State Machine 0

// Create a button object for the encoder
// Connect your button between the GPIO pin and GND
// Library uses internal pullup resistors of PICO


// Create the Audio Buffer
audio_buffer_pool_t *ap;

// Create the Oled screen
u8g2_t u8g2; 

mdaEPiano ep(96);
MIDIInputUSB usbmidi;
   
		
    volatile long dsp_start;
    volatile long dsp_end;
	
    // This task prints the statistics about the running FreeRTOS tasks
    // and how long it takes for the I2S callback to run.
    void print_task(void *p)
    {
        char ptrTaskList[2048];
        while (1)
        {
            vTaskList(ptrTaskList);
            printf("\033[2J");
            printf("\033[0;0HTask\t\tState\tPrio\tStack\tNum\n%s\n", ptrTaskList);
            printf("======================================================\n");
            printf("B = Blocked, R = Ready, D = Deleted, S = Suspended\n");
            printf("Milliseconds since boot: %d\n", xTaskGetTickCount() * portTICK_PERIOD_MS);
            printf("dsp task took %d uS\n", dsp_end - dsp_start);
            watchdog_update();
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }

    // MIDI callbacks
    void note_on_callback(uint8_t note, uint8_t level, uint8_t channel)
    {
        if (level > 0)
        {
            ep.noteOn(note, level);
	    gpio_put(25, 1);
        }
        else
        {
            ep.noteOff(note);
	    gpio_put(25, 0);
        }
    }

    void note_off_callback(uint8_t note, uint8_t level, uint8_t channel)
    {
        ep.noteOff(note);
	gpio_put(25, 0);
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
	
	// Test the encoder settings
    void bt_task(void *p)
    {
		while (1)
        {
        }
    }
	
	// Test the encoder settings
    void gui_task(void *p)
    {	
		int8_t res;
		
		u8g2_InitDisplay(&u8g2);
		u8g2_SetPowerSave(&u8g2, 0); 	
        u8g2_ClearBuffer(&u8g2);
        u8g2_ClearDisplay(&u8g2);
		u8g2_SetDrawColor(&u8g2, 1);		
		u8g2_SetFont(&u8g2, u8g2_font_luBS10_tf  );		
		
		while (1)
		{
			// Select Program
			res = pico_UserInterfaceProgramSelect(&u8g2, &enc, &ep);			
			
			//res = pico_UserInterfaceSelectionList(&u8g2, &enc, "Program", 1, "abcdef\nghijkl\nmnopqr");
			if (res == 1) {
				vTaskDelay(pdMS_TO_TICKS(500));
				pico_UserInterfaceInputValue(&u8g2, &enc, "Volume", "", 0, 0, 100, 3, "db");
			}
			vTaskDelay(pdMS_TO_TICKS(500));
			
		
			pico_UserInterfaceSelectionList(&u8g2, &enc, "test2", 1, "eins\nzwei\ndrei\nvier\nfuenf\nsechs\nsieben");
			vTaskDelay(pdMS_TO_TICKS(500));
		}
	}

    int main(void)
    {
        // initialize the hardware
        pico_init();

        // Initialize the audio subsystem
        ap = init_audio();
				
		ep.setVolume(64);
					
		// Initialize the display u8g2_Setup_sh1106_128x64_noname_f(
		//oled_init(i2c1, 0x3C, 128, 64);
		u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, 
										  U8G2_R0, 
										  u8x8_byte_pico_hw_i2c, 
										  u8x8_gpio_and_delay_pico);	   
			

		// Create FreeRTOS Tasks for USB MIDI and printing statistics
        xTaskCreate(usb_midi_task, "USBMIDI", 4096, NULL, configMAX_PRIORITIES, NULL);
        xTaskCreate(print_task, "TASKLIST", 1024, NULL, configMAX_PRIORITIES - 1, NULL);
		xTaskCreate(gui_task, "gui_task", 4096, NULL, configMAX_PRIORITIES - 1, NULL);
		//xTaskCreate(gui_task, "bt_task", 1024, NULL, configMAX_PRIORITIES - 1, NULL);
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

        dsp_start = to_us_since_boot(get_absolute_time());

		// Process the synth
		ep.process(&l[0], &r[0]);

        // We are filling the buffer with 32-bit samples (2 channels)
        for (uint i = 0; i < buffer->max_sample_count; i++)
        {
	    samples[i * 2 + 0] = l[i] << 16;  // LEFT
            samples[i * 2 + 1] = r[i] << 16; // RIGHT
        }

        dsp_end = to_us_since_boot(get_absolute_time());

        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(ap, buffer);
        return;
    }

#ifdef __cplusplus
}
#endif
