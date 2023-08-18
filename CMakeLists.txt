cmake_minimum_required(VERSION 3.11)

project(jtagpp)

option(JTAGPP_STANDALONE_BUILD "Use pre-built libftdi and libusb")
option(JTAGPP_NO_VENDOR_DATABASE "Do not build in JEDEC JEP106 vendor database")

if(JTAGPP_STANDALONE_BUILD)
    set(LibFTDI1_DIR ${CMAKE_SOURCE_DIR}/3rdparty/cmake)
    set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/3rdparty/cmake ${CMAKE_MODULE_PATH})
endif()

if(JTAGPP_NO_VENDOR_DATABASE)
    add_definitions(-DNO_VENDOR_DATABASE)
endif()

find_package(LibFTDI1 NO_MODULE REQUIRED)

add_subdirectory(jtagpp)
add_subdirectory(tools)
