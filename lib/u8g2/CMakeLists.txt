include(FetchContent)

FetchContent_Declare(
  u8g2
  GIT_REPOSITORY  https://github.com/olikraus/u8g2.git
)
		   		   
FetchContent_MakeAvailable(u8g2)

target_include_directories(u8g2 PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/csrc
)

target_link_libraries(u8g2)

target_compile_options(u8g2 PUBLIC
	-Wno-deprecated-declarations
	-Wno-unused-variable
	-Wno-unused-function
	-Wno-unused-parameter
	-Wno-write-strings
	-Wno-narrowing
)
