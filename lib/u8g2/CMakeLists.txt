file(GLOB COMPONENT_SRCS ${CMAKE_CURRENT_LIST_DIR}/u8g2/csrc/*.c)

add_library(u8g2 STATIC ${COMPONENT_SRCS})

target_include_directories(u8g2 PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/u8g2/csrc
)

target_link_libraries(u8g2 pico_stdlib)

target_compile_options(u8g2 PUBLIC
	-Wno-deprecated-declarations
	-Wno-unused-variable
	-Wno-unused-function
	-Wno-unused-parameter
	-Wno-write-strings
	-Wno-narrowing
)
