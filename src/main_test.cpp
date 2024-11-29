#include <stdio.h>
#include "pico/binary_info.h"
#include "project_config.h"
#include "pico_hw.h"

#if __has_include("bsp/board_api.h")
#include "bsp/board_api.h"
#else
#include "bsp/board.h"
#endif

#include "FreeRTOS.h"
#include <task.h>
#include <queue.h>


#ifdef __cplusplus
extern "C"
{
#endif

    // This task blinks the LEDs on PIN_LED
    void blinker_task(void *pvParameters)
    {
        while (1)
        {
            gpio_put(PIN_LED, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
            gpio_put(PIN_LED, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }

    int main(void)
    {
    
        // initialize the hardware
        pico_init();    // Board    
                                            
        /*while (1) {       
            gpio_put(25, 1);
            sleep_ms(500);
            gpio_put(25, 0);
            sleep_ms(500);
        }*/

        // Create FreeRTOS Tasks for USB MIDI and printing statistics
        xTaskCreate(blinker_task, "BLINKER", 128, NULL, configMAX_PRIORITIES - 1, NULL);

        // Start the scheduler.
        vTaskStartScheduler();

        // Idle loop.
        while (1)
        {
            ;
            ;
        } 
    }

#ifdef __cplusplus
}
#endif
