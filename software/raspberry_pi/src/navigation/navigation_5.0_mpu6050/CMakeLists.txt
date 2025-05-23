cmake_minimum_required(VERSION 3.10)
project(run_auv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

# Adiciona o diretório de include da Eigen
find_path(EIGEN3_INCLUDE_DIR Eigen/Dense)
if(NOT EIGEN3_INCLUDE_DIR)
    message(FATAL_ERROR "Eigen3 não encontrado. Instale a Eigen3 ou defina EIGEN3_INCLUDE_DIR.")
endif()
include_directories(${EIGEN3_INCLUDE_DIR})

find_package(MAVSDK REQUIRED)

find_library(WIRINGPI_LIB wiringPi)

set(SOURCES
    src/navigation.cpp
    src/sensors.cpp
    src/thrustersctrl.cpp
    src/yoloctrl.cpp
    src/auverror.cpp
    src/utils.cpp
    src/activator.cpp
)

add_executable(run_auv ${SOURCES})

target_link_libraries(run_auv MAVSDK::mavsdk ${WIRINGPI_LIB} pthread)
