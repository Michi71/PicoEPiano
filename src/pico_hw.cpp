#pragma GCC optimize("Ofast")
#include <pico/time.h>
#include "hardware/clocks.h"
#include <pico/stdlib.h>
#include <hardware/vreg.h>
#include <pico/multicore.h>

#include "pico_hw.h"
#include "project_config.h"

#if PICO_RP2040
// #include "../../memops_opt/memops_opt.h"
#else
#include <hardware/structs/qmi.h>
#include <hardware/structs/xip.h>
#endif

uint8_t u8x8_byte_pico_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[32]; /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t buf_idx;
    uint8_t *data;

    switch (msg)
    {
    case U8X8_MSG_BYTE_SEND:
        data = (uint8_t *)arg_ptr;
        while (arg_int > 0)
        {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
        break;
    case U8X8_MSG_BYTE_INIT:
        i2c_init(i2c1, 400*1000);
        gpio_set_function(PIN_OLED_SDA, GPIO_FUNC_I2C);
        gpio_set_function(PIN_OLED_SCL, GPIO_FUNC_I2C);
        gpio_pull_up(PIN_OLED_SDA);
        gpio_pull_up(PIN_OLED_SCL);
        break;
    case U8X8_MSG_BYTE_SET_DC:
        break;
    case U8X8_MSG_BYTE_START_TRANSFER:
        buf_idx = 0;
        break;
    case U8X8_MSG_BYTE_END_TRANSFER:
        i2c_write_blocking(i2c1, u8x8_GetI2CAddress(u8x8) >> 1, buffer, buf_idx, false);
        break;
    default:
        return 0;
        break;
    }
    return 1;
}

uint8_t u8x8_gpio_and_delay_pico(u8x8_t *u8x8, uint8_t msg,uint8_t arg_int, void *arg_ptr) 
{
  return 1;
}

void pico_init()
{
#if PICO_RP2350
    volatile uint32_t *qmi_m0_timing=(uint32_t *)0x400d000c;
    vreg_disable_voltage_limit();
    vreg_set_voltage(VREG_VOLTAGE_1_60);
    sleep_ms(10);
    *qmi_m0_timing = 0x60007204;
    set_sys_clock_hz(444000000, 0);
    *qmi_m0_timing = 0x60007303;
#else
    memcpy_wrapper_replace(NULL);
    hw_set_bits(&vreg_and_chip_reset_hw->vreg, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);
    sleep_ms(33);
    set_sys_clock_khz(402 * 1000, true);
#endif

    // Initialize TinyUSB
    board_init();
    tusb_init();
    stdio_init_all();

    // LED on GPIO25
    gpio_init(PIN_LED);
    gpio_set_dir(PIN_LED, GPIO_OUT);

    uint32_t rand_seed;
    for (int i = 0; i < 32; i++)
    {
        bool randomBit = rosc_hw->randombit;
        rand_seed = rand_seed | (randomBit << i);
    }

    srand(rand_seed);
}
