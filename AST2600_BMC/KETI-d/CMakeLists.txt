cmake_minimum_required(VERSION 3.03)

set(
    OUTPUT
    KETI-DBus
    #dbus_client
)
set(
    SOURCE_FILES
    keti_dbus.cpp
    smbus.cpp
    #dbus_client.cpp
    )

set(ROOT_DIR /home/keti/BMC_SDK)
set(DBus_INCLUDE_DIR ${ROOT_DIR}/output/build/dbus-cpp-0.9.0/include)
set(DBus_LIBRARY_DIR ${ROOT_DIR}/output/build/dbus-cpp-0.9.0/src/libs)
set(DBus_INC_DIR /home/keti/BMC_SDK/source/AST2600_BMC/include)
set(Buildroot_INCLUDE_DIR ${ROOT_DIR}/output/host/usr/arm-buildroot-linux-gnueabihf/sysroot/usr/include)
set(Buildroot_LIBRARY_DIR ${ROOT_DIR}/output/host/usr/arm-buldroot-linux-gnueabihf/sysroot/usr/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY /home/keti/BMC_SDK/source/AST2600_BMC/output/bin)
# Compiler setting
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CROSS_COMPILE ${ROOT_DIR}/output/host/usr/bin/arm-linux-)
set(GCC ${CROSS_COMPILE}gcc)
set(GPP ${CROSS_COMPILE}g++)
set(CMAKE_C_COMPILER ${GCC})
set(CMAKE_CXX_COMPILER ${GPP})
set(CMAKE_CXX_FLAGS " -Wall -w -DBOOST_LOG_DYN_LINK -Wno-psabi -fpermissive -DMG_ENABLE_SSL -DMG_ENABLE_THREADS -DMG_ENABLE_HTTP_WEBSOCKET=0 -DMG_ENABLE_HTTP_STREAMING_MULTIPART -pthread -std=c++11") 
set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g")
message(STATUS "Compiler")
message(STATUS " - ID       \t: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " - Version  \t: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " - Path     \t: ${CMAKE_CXX_COMPILER}")
project(dbus-server  VERSION 1.1.0 LANGUAGES CXX)

include_directories(
    ${DBus_INCLUDE_DIR}
    ${DBus_INC_DIR}
    ${Buildroot_INCLUDE_DIR}
    )

# Source
link_directories(
    ${DBus_LIBRARY_DIR}
    ${Buildroot_LIBRARY_DIR}
)

add_executable(${OUTPUT}  ${SOURCE_FILES})

target_link_libraries( ${OUTPUT}
    crypto 
    ssl 
    m
    rt
    uuid
    pthread
    pam
    pam_misc
    cpprest
    stdc++fs
    libdbus-c++-1.so.0
)

