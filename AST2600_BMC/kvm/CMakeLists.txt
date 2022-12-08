# CMake version
cmake_minimum_required(
    VERSION 3.2
)
set(
    OUTPUT
    KETI-KVM
)
set(
    SOURCE_FILES
    ast-video.c
    video.c
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
set(CMAKE_CXX_FLAGS "-O1 -Wall -Wfatal-errors -w -DBOOST_LOG_DYN_LINK -Wno-psabi -fpermissive -DMG_ENABLE_SSL -DMG_ENABLE_THREADS -DMG_ENABLE_HTTP_WEBSOCKET=0 -DMG_ENABLE_HTTP_STREAMING_MULTIPART -pthread -std=c++11 --Wimplicit-function-declaration") 
set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g")
message(STATUS "Compiler")
message(STATUS " - ID       \t: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " - Version  \t: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " - Path     \t: ${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_FLAGS "-lc -lgcc -lm -O3 -g -w -Wall -lz -std=c++11")

add_executable(${OUTPUT} ${SOURCE_FILES})
target_link_libraries(
    ${OUTPUT}
    pthread
)
