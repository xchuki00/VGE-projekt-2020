cmake_minimum_required(VERSION 3.5)
project(vge)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_subdirectory(lib/nanogui)
include_directories(lib/nanogui/include)
include_directories(${NANOGUI_EXTRA_INCS})
add_definitions(${NANOGUI_EXTRA_DEFS})

include_directories(./include ./src)

file(GLOB main_SRC
        "./include/*.h"
        "./src/*.cpp"
        )
add_executable(vge Vge.cpp ${main_SRC})
target_link_libraries(vge nanogui ${NANOGUI_EXTRA_LIBS})
