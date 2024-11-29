#include "pico_hw.h"
#include "project_config.h"

#define clockspeed 360000

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
        // Overclock the CPU to 402 MHz.
        /*vreg_set_voltage(VREG_VOLTAGE_1_30);
        sleep_ms(100);
        set_sys_clock_khz(366000, true);
	clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                    366000,
                    366000);*/

	/*vreg_set_voltage(VREG_VOLTAGE_1_30);
	sleep_ms(10);
	set_sys_clock_khz(402000, true);*/

	// A0 SDK won't pick up on the PICO_EMBED_XIP_SETUP flag, so just to make sure:
	/*hw_write_masked(
		&qmi_hw->m[0].timing,
		3 << QMI_M0_TIMING_RXDELAY_LSB | 2 << QMI_M0_TIMING_CLKDIV_LSB,
		QMI_M0_TIMING_RXDELAY_BITS | QMI_M0_TIMING_CLKDIV_BITS
	);*/
	vreg_disable_voltage_limit ();
        vreg_set_voltage(VREG_VOLTAGE_1_40);
        sleep_ms(10);
        set_sys_clock_khz(clockspeed, false);
        clock_configure(
                clk_peri,
                0,                                                // No glitchless mux
                CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
                clockspeed * 1000,                                // Input frequency
                clockspeed * 1000                                 // Output (must be same as no divider)
        ); 

        // Initialize TinyUSB
        board_init();
        tusb_init();
        stdio_init_all();
	
        // LED on GPIO25
        gpio_init(PIN_LED);
        gpio_set_dir(PIN_LED, GPIO_OUT);

	gpio_set_function(47, GPIO_FUNC_XIP_CS1); // CS for PSRAM
	xip_ctrl_hw->ctrl|=XIP_CTRL_WRITABLE_M1_BITS;
	
        uint32_t rand_seed;
        for (int i = 0; i < 32; i++)
        {
                bool randomBit = rosc_hw->randombit;
                rand_seed = rand_seed | (randomBit << i);
        }

        srand(rand_seed);
}
