cmake_minimum_required(VERSION 3.10)
project(run_pix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

find_package(MAVSDK REQUIRED)

set(SOURCES
	src/data_pix.cpp
)

add_executable(run_pix ${SOURCES})

target_link_libraries(run_pix MAVSDK::mavsdk)