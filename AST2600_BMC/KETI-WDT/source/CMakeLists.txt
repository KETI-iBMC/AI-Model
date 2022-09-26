# CMake version
cmake_minimum_required(
    VERSION 3.2
)
set(
    OUTPUT
    KETI-WDT
)
set(
    SOURCE_FILES
    main.cpp
    KETI-WDT.cpp
    KETI-FILE.cpp
    WDT_stdx.cpp
    

)

# Directory setting
set(ROOT_DIR /home/keti/BMC_SDK)
set(Boost_INCLUDE_DIR ${ROOT_DIR}/BMC_SDK/output/build/boost-1.73.0)
set(Boost_LIBRARY_DIR ${ROOT_DIR}/BMC_SDK/output/build/boost-1.73.0/stage/lib)
set(PROJECT_DIR ${ROOT_DIR}/source/AST2600_BMC)
set(WDT_DIR ${PROJECT_DIR}/KETI-WDT)
set(INC_DIR ${WDT_DIR}/include)


# Include
include_directories(
    ${Boost_INCLUDE_DIR}
    ${INC_DIR}
)
## Source
link_directories(
    ${Boost_LIBRARY_DIR} 
)

add_executable(${OUTPUT} ${SOURCE_FILES})
target_link_libraries(
    ${OUTPUT}
    pthread
)