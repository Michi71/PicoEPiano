# Pico CMakeLists.txt

cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_EXTRAS_PATH "../lib/pico-extras")

# set(PICO_PLATFORM "rp2040")
# set(PICO_BOARD "pico")
# set(PICO_BOARD waveshare_rp2040_plus_16mb)
set(PICO_PLATFORM "rp2350-arm-s") 
# set(PICO_BOARD "sparkfun_promicro_rp2350")
set(PICO_BOARD "solderparty_rp2350_stamp_xl")

set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-Kernel")
set(PICO_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--print-memory-usage")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--print-memory-usage")

project(main C CXX ASM)
enable_language(CXX)
SET(PROJECT_NAME "main")
SET(BUILD_NAME "${PROJECT_NAME}")

pico_sdk_init()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR} ".")

add_subdirectory(lib/audio)
add_subdirectory(lib/encoder)
add_subdirectory(lib/u8g2)

add_compile_options(-flto -fwhole-program -ffast-math -ffunction-sections -fdata-sections -fms-extensions -O2)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

# set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${BUILD_NAME}")

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

add_executable(${PROJECT_NAME}
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

pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
if (PICO_PLATFORM STREQUAL "rp2040")
    SET(BUILD_NAME "${BUILD_NAME}-RP2040")
#    add_subdirectory(memops_opt)
#    target_link_libraries(${PROJECT_NAME} PRIVATE memops_opt)
    target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
else()
    SET(BUILD_NAME "${BUILD_NAME}-RP2350")
    target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=3)
endif ()
pico_set_boot_stage2(${PROJECT_NAME} slower_boot2)
    

pico_set_program_name(${PROJECT_NAME} "main")
pico_set_program_version(${PROJECT_NAME} "0.1")

pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 0)

# Add the standard library to the build
target_link_libraries(${PROJECT_NAME} pico_stdlib)

# Add any user requested libraries
target_link_libraries(${PROJECT_NAME}
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
	target_compile_definitions(${PROJECT_NAME} PRIVATE
                USE_AUDIO_I2S=1
                PICO_USE_SW_SPIN_LOCKS=1
                PICO_AUDIO_I2S_DATA_PIN=26
                PICO_AUDIO_I2S_CLOCK_PIN_BASE=27	 
		)
else()
	target_compile_definitions(${PROJECT_NAME} PRIVATE
                USE_AUDIO_I2S=1
                PICO_AUDIO_I2S_DATA_PIN=26
                PICO_AUDIO_I2S_CLOCK_PIN_BASE=27
		)
endif()


target_link_options(${PROJECT_NAME} PRIVATE
        -Xlinker --print-memory-usage
)

file(GLOB files "${CMAKE_CURRENT_LIST_DIR}/piosrc/*.pio")
foreach(file ${files})
    message ("Adding ${file} to PIO build.")
    pico_generate_pio_header(${PROJECT_NAME} ${file})
endforeach()

add_compile_options("-Wall" "-Wredundant-decls")

pico_enable_stdio_usb(${PROJECT_NAME} 0)    # 0: disable, 1: enable
pico_enable_stdio_uart(${PROJECT_NAME} 1)   # 0: disable, 1: enable

pico_add_extra_outputs(${PROJECT_NAME})
