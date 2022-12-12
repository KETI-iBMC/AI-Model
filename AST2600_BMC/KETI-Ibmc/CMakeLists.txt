# Variable definition
## Execute file
set(
    OUTPUT
    KETI-Ibmc
)

## Source file
set(
    SOURCE_FILES 
    msg_session.cpp
    rmcp.cpp
    khandler.cpp
    keti-Ibmc.cpp
    handler.cpp
    hwcontrol.cpp
    init.cpp
    lssdp.cpp
    
    ntp.cpp
    ethernetinterface.cpp
    eventservice.cpp
    logservice.cpp
)

# Directory setting
set(Boost_INCLUDE_DIR ${ROOT_DIR}/BMC_SDK/output/build/boost-1.73.0)
set(Boost_LIBRARY_DIR ${ROOT_DIR}/BMC_SDK/output/build/boost-1.73.0/stage/lib)
set(DBus_INCLUDE_DIR ${ROOT_DIR}/output/build/dbus-cpp-0.9.0/include)
set(DBus_LIBRARY_DIR ${ROOT_DIR}/output/build/dbus-cpp-0.9.0/src/libs)
# Linkinking
## Header -> lib
include_directories(
    ${Boost_INCLUDE_DIR}
    ${DBus_INCLUDE_DIR}
    ${INC_DIR}
)
## Source
link_directories(
    ${Boost_LIBRARY_DIR} 
    ${DBus_LIBRARY_DIR}

)

# Build
add_executable(${OUTPUT} ${SOURCE_FILES})
target_link_libraries( ${OUTPUT}
    crypto 
    ssl 
    m
    rt
    uuid
    pthread
    pam
    pam_misc
    boost_system
    boost_log
    boost_log_setup
    boost_thread
    cpprest
    sqlite3
    stdc++fs
    libdbus-c++-1.so.0
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libapps.a
)

