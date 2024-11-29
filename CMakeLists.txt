# Pico CMakeLists.txt

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_EXTRAS_PATH "../lib/pico-extras")

# set(PICO_PLATFORM "rp2040")
# set(PICO_BOARD "pico")
# set(PICO_BOARD waveshare_rp2040_plus_16mb)
set(PICO_PLATFORM "rp2350-arm-s") 
# set(PICO_BOARD "sparkfun_promicro_rp2350")
set(PICO_BOARD "solderparty_rp2350_stamp_xl")

set(PICO_NO_COPRO_DIS 1)

set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-Kernel")
set(PICO_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

project(main C CXX ASM)

pico_sdk_init()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR} ".")

add_subdirectory(lib/audio)
add_subdirectory(lib/encoder)
add_subdirectory(lib/u8g2)

# Set the real flag based on the PICO_PLATFORM variable
if (DEFINED PICO_PLATFORM AND PICO_PLATFORM STREQUAL "rp2350-arm-s")
    set(REAL_TYPE_FLAG "float")
    message("Using float for real type")
else()
    set(REAL_TYPE_FLAG "fixed")
    message("Using fixed for real type")
endif()

if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message("Skipping FreeRTOS examples as FREERTOS_KERNEL_PATH not defined")
    return()
endif()

add_executable(main
    src/usb_descriptors.c
    src/mdaEPiano.cpp
    src/main.cpp 
    src/pico_hw.cpp
    src/midi_input_usb.cpp
    src/get_serial.c
    src/pico_selection_list.cpp
    src/pico_input_value.cpp
    src/pico_program_select.cpp
    src/pico_param_select.cpp
    lib/FreeRTOS-Kernel/portable/MemMang/heap_3.c)

pico_set_program_name(main "main")
pico_set_program_version(main "0.1")

pico_enable_stdio_uart(main 1)
pico_enable_stdio_usb(main 0)
pico_add_extra_outputs(main)

# Add the standard library to the build
target_link_libraries(main pico_stdlib)

# Add any user requested libraries
target_link_libraries(main
        hardware_adc
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_watchdog
        hardware_clocks
        hardware_sync_spin_lock
        hardware_sync
        pico_unique_id
        pico_bootsel_via_double_reset
        pico_util_buffer
        pico_multicore
        pico_stdio_uart
        tinyusb_device
        tinyusb_board
        Audio
        RotaryEncoder
        u8g2
        FreeRTOS-Kernel
        )


if (DEFINED PICO_PLATFORM AND PICO_PLATFORM STREQUAL "rp2350-arm-s")
	target_compile_definitions(main PRIVATE
                USE_AUDIO_I2S=1
                PICO_USE_SW_SPIN_LOCKS=1
                PICO_AUDIO_I2S_DATA_PIN=26
                PICO_AUDIO_I2S_CLOCK_PIN_BASE=27	 
		)
else()
	target_compile_definitions(main PRIVATE
                USE_AUDIO_I2S=1
                PICO_AUDIO_I2S_DATA_PIN=26
                PICO_AUDIO_I2S_CLOCK_PIN_BASE=27
		)
endif()


target_link_options(main PRIVATE
        -Xlinker --print-memory-usage
)

file(GLOB files "${CMAKE_CURRENT_LIST_DIR}/piosrc/*.pio")
foreach(file ${files})
    message ("Adding ${file} to PIO build.")
    pico_generate_pio_header(main ${file})
endforeach()

add_compile_options("-Wall" "-Wredundant-decls")

# All code and data must be RAM-resident so we can stream continuously from
# flash using direct SSI DMA, which would trample on XIP.
# pico_set_binary_type(main copy_to_ram)
pico_set_binary_type(main no_flash)


if (PICO_PLATFORM STREQUAL "rp2040")
	pico_define_boot_stage2(main_boot2 ${PICO_SDK_PATH}/src/rp2040/boot_stage2/boot2_w25q080.S)
	pico_set_boot_stage2(main main_boot2)
	target_compile_definitions(main_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
else ()
	target_compile_definitions(main PRIVATE
		PICO_EMBED_XIP_SETUP=1
		PICO_BOOT_STAGE2_CHOOSE_W25Q080=1
		PICO_FLASH_SPI_CLKDIV=4
		PICO_FLASH_SPI_RXDELAY=3
		)
endif()

target_compile_definitions(main PRIVATE	PICO_FLASH_SPI_CLKDIV=4)
target_compile_definitions(main PRIVATE PICO_STACK_SIZE=0x200)

#if (PICO_PLATFORM STREQUAL "rp2040")
#    pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
#    target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
#    pico_set_boot_stage2(main slower_boot2)
#else()
#    target_compile_definitions(main PRIVATE
#		PICO_EMBED_XIP_SETUP=1
#		PICO_BOOT_STAGE2_CHOOSE_W25Q080=1
#		PICO_FLASH_SPI_CLKDIV=4
#		PICO_FLASH_SPI_RXDELAY=4
#                PICO_CLOCK_AJDUST_PERI_CLOCK_WITH_SYS_CLOCK=1
#		)
# endif()

pico_add_extra_outputs(main)
