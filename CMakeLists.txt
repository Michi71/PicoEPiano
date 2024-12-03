# Pico CMakeLists.txt

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_EXTRAS_PATH "../lib/pico-extras")

set(PICO_PLATFORM "rp2040")
# set(PICO_BOARD "pico")
set(PICO_BOARD waveshare_rp2040_plus_16mb)
# set(PICO_PLATFORM "rp2350-arm-s") 
# set(PICO_BOARD "sparkfun_promicro_rp2350")
# set(PICO_BOARD "solderparty_rp2350_stamp_xl")

set(PICO_NO_COPRO_DIS 1)

set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-Kernel")
set(PICO_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--print-memory-usage")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--print-memory-usage")

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.0.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.0.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# If you want debug output from USB (pass -DPICO_STDIO_USB=1) this ensures you don't lose any debug output while USB is set up
if (NOT DEFINED PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS)
    set(PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 3000)
endif()

project(main C CXX ASM)

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

# Set all global variables here
set(OVERCLOCK_ENABLED 1)    # 1: enable, 0: disable

if(OVERCLOCK_ENABLED)

    message(WARNING "Overclocking is enabled. This may damage your device. Use at own risk.")

    if(${PICO_BOARD} STREQUAL "pico" OR ${PICO_PLATFORM} STREQUAL "rp2040")
        # Overclocking profiles
        #      SYS_CLK  | FLASH_CLK | Voltage
        #  1  | 240MHz  |  120MHZ   |  1.10(V) (default, stable, recommended for most devices)
        #  2  | 266MHz  |  133MHz   |  1.10(V)
        #  3  | 360MHz  |  90MHz    |  1.20(V)
        #  4  | 400MHz  |  100MHz   |  1.30(V)
        #  5  | 416MHz  |  104MHz   |  1.30(V)
        set(OVERCLOCK_PROFILE 4)

        if(OVERCLOCK_PROFILE EQUAL 1)
            set(SYS_CLK_KHZ 240000)             # CPU clock speed
            set(PERI_CLK_KHZ ${SYS_CLK_KHZ})    # Peripheral clock speed
        elseif(OVERCLOCK_PROFILE EQUAL 2)
            set(SYS_CLK_KHZ 266000)
            set(PERI_CLK_KHZ ${SYS_CLK_KHZ})
        elseif(OVERCLOCK_PROFILE EQUAL 3)
            set(SYS_CLK_KHZ 360000)
            set(PERI_CLK_KHZ ${SYS_CLK_KHZ})
        elseif(OVERCLOCK_PROFILE EQUAL 4)
            set(SYS_CLK_KHZ 400000)
            set(PERI_CLK_KHZ ${SYS_CLK_KHZ})
        elseif(OVERCLOCK_PROFILE EQUAL 5)
            set(SYS_CLK_KHZ 416000)
            set(PERI_CLK_KHZ ${SYS_CLK_KHZ})
        else()
            message(FATAL_ERROR "Invalid overclocking profile")
        endif()
    elseif(${PICO_BOARD} STREQUAL "pico2" OR ${PICO_PLATFORM} STREQUAL "rp2350")
        # Overclocking profiles
        #      SYS_CLK  | FLASH_CLK | Voltage
        #  1  | 366MHz  |  183MHz   |  1.20(V)
        set(OVERCLOCK_PROFILE 1)

        if(OVERCLOCK_PROFILE EQUAL 1)
            set(SYS_CLK_KHZ 366000)             # CPU clock speed
            set(PERI_CLK_KHZ ${SYS_CLK_KHZ})    # Peripheral clock speed
        else()
            message(FATAL_ERROR "Invalid overclocking profile")
        endif()
    endif()

else()  # OVERCLOCK_ENABLED
    message(WARNING "Overclocking is disabled.")

    if(${PICO_BOARD} STREQUAL "pico" OR ${PICO_PLATFORM} STREQUAL "rp2040")
        set(SYS_CLK_KHZ 125000) # CPU clock speed
        set(PERI_CLK_KHZ ${SYS_CLK_KHZ})    # Peripheral clock speed
    elseif(${PICO_BOARD} STREQUAL "pico2" OR ${PICO_PLATFORM} STREQUAL "rp2350")
        set(SYS_CLK_KHZ 150000) # CPU clock speed
        set(PERI_CLK_KHZ ${SYS_CLK_KHZ})    # Peripheral clock speed
    endif()

endif() # OVERCLOCK_ENABLED

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


#if (PICO_PLATFORM STREQUAL "rp2040")
#	pico_define_boot_stage2(main_boot2 ${PICO_SDK_PATH}/src/rp2040/boot_stage2/boot2_w25q080.S)
#	pico_set_boot_stage2(main main_boot2)
#	target_compile_definitions(main_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
#else ()
#	target_compile_definitions(main PRIVATE
#                PICO_EMBED_XIP_SETUP=1
#                PICO_FLASH_SPI_RXDELAY=3
#                PICO_FLASH_SPI_CLKDIV=4
#                PICO_CLOCK_AJDUST_PERI_CLOCK_WITH_SYS_CLOCK=1
#		)
#endif()

#target_compile_definitions(main PRIVATE	PICO_FLASH_SPI_CLKDIV=4)
#target_compile_definitions(main PRIVATE PICO_STACK_SIZE=0x200)

# add target common defines here
target_compile_definitions(main PUBLIC DEFAULT_SYS_CLK_KHZ=${SYS_CLK_KHZ})
target_compile_definitions(main PUBLIC DEFAULT_PERI_CLK_KHZ=${PERI_CLK_KHZ})

# Note: If you are using a NOR flash like "w25q16". Just keep the following content.
# The maximum speed of "w25q16" is 133MHz, However, the clock speed of XIP QSPI is divided from "sys_clk".
# So, when your "sys_clk" is greater than 266MHz and default PICO_FLASH_SPI_CLKDIV=2, It will exceed the
# maximum speed, because PICO_FLASH_SPI_CLKDIV must be even, So 4 is good for most purpose, Otherwise,
# nothing should be done. These things will only useful when you overclocking the rp2040.
if(${PICO_BOARD} STREQUAL "pico" OR ${PICO_PLATFORM} STREQUAL "rp2040")
    if(${SYS_CLK_KHZ} GREATER 266000)
        set(PICO_FLASH_SPI_CLKDIV 4)
    else()
        set(PICO_FLASH_SPI_CLKDIV 2)
    endif()
elseif(${PICO_BOARD} STREQUAL  "pico2" OR ${PICO_PLATFORM} STREQUAL  "rp2350-arm-s")
    if(${SYS_CLK_KHZ} GREATER 366000)
        set(PICO_FLASH_SPI_CLKDIV 4)
    else()
        set(PICO_FLASH_SPI_CLKDIV 2)
    endif()
endif()

math(EXPR FLASH_CLK_KHZ "${SYS_CLK_KHZ} / ${PICO_FLASH_SPI_CLKDIV}")
math(EXPR FLASH_CLK_MHZ "${FLASH_CLK_KHZ} / 1000")
math(EXPR SYS_CLK_MHZ "${SYS_CLK_KHZ} / 1000")
message(WARNING "
    CPU speed   : ${SYS_CLK_MHZ} MHz
    Flash speed : ${FLASH_CLK_MHZ} MHz
")
target_compile_definitions(bs2_default PRIVATE PICO_FLASH_SPI_CLKDIV=${PICO_FLASH_SPI_CLKDIV})
target_compile_definitions(${PROJECT_NAME} PRIVATE FLASH_CLK_KHZ=${FLASH_CLK_KHZ})

pico_enable_stdio_usb(${PROJECT_NAME} 0)    # 0: disable, 1: enable
pico_enable_stdio_uart(${PROJECT_NAME} 1)   # 0: disable, 1: enable

pico_add_extra_outputs(main)
