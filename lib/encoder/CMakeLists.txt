add_library(Encoder STATIC
	src/encoder.cpp
	src/push_button.cpp
)

target_include_directories(Encoder PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/include
)

pico_generate_pio_header(Encoder ${CMAKE_CURRENT_LIST_DIR}/piosrc/encoder.pio)

target_link_libraries(Encoder pico_stdlib hardware_pio)
