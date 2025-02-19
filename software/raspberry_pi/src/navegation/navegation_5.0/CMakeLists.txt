cmake_minimum_required(VERSION 3.10)
project(run_auv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

find_package(MAVSDK REQUIRED)

find_library(WIRINGPI_LIB wiringPi)

set(SOURCES
	src/main_navegation.cpp
)

add_executable(run_auv ${SOURCES})

target_link_libraries(run_auv MAVSDK::mavsdk ${WIRINGPI_LIB} pthread)
