# CMake version
cmake_minimum_required(
    VERSION 3.2
)

project(
    KETI-WDT
    LANGUAGES CXX
    VERSION 1.0.0
)


# Compiler setting
set(ROOT_DIR /home/keti/BMC_SDK)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CROSS_COMPILE ${ROOT_DIR}/output/host/usr/bin/arm-linux-)
set(GCC ${CROSS_COMPILE}gcc)
set(GPP ${CROSS_COMPILE}g++)
set(CMAKE_C_COMPILER ${GCC})
set(CMAKE_CXX_COMPILER ${GPP})

message(STATUS "Compiler")
message(STATUS " - ID       \t: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " - Version  \t: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " - Path     \t: ${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_FLAGS " -lcpprest -lcrypto -lm -lrt -lpthread -Wall -DBOOST_LOG_DYN_LIN -g -w -fpermissive -w -DMG_ENABLE_SSL -pthread -g -std=c++11") 

# Directory setting
set(Boost_INCLUDE_DIR ${ROOT_DIR}/BMC_SDK/output/build/boost-1.73.0)
set(Boost_LIBRARY_DIR ${ROOT_DIR}/BMC_SDK/output/build/boost-1.73.0/stage/lib)
set(PROJECT_DIR ${ROOT_DIR}/source/AST2600_BMC)
set(WDT_DIR ${PROJECT_DIR}/KETI-WDT)
set(INC_DIR ${WDT_DIR}/include)
set(OUTPUT ../output/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DIR}/output/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_DIR}/output/libs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/output/libs)

# Include
include_directories(
    ${Boost_INCLUDE_DIR}
    ${INC_DIR}
)

## Source
link_directories(
    ${Boost_LIBRARY_DIR} 
)

add_subdirectory(${WDT_DIR}/source)

