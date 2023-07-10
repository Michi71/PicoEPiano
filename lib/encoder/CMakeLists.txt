add_library(RotaryEncoder STATIC
	src/rotary_encoder.cpp
)

target_include_directories(RotaryEncoder PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/include
)

pico_generate_pio_header(RotaryEncoder ${CMAKE_CURRENT_LIST_DIR}/piosrc/button_debounce.pio)
pico_generate_pio_header(RotaryEncoder ${CMAKE_CURRENT_LIST_DIR}/piosrc/rotary_encoder.pio)

target_link_libraries(RotaryEncoder pico_stdlib hardware_pio hardware_irq)
