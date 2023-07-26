# SPDX-License-Identifier: Apache-2.0

# Configures binary tools as host GNU binutils

find_program(CMAKE_OBJCOPY objcopy)
find_program(CMAKE_OBJDUMP objdump)
find_program(CMAKE_AR      ar     )
find_program(CMAKE_RANLILB ranlib )
find_program(CMAKE_READELF readelf)

find_program(CMAKE_GDB     gdb    )

# Include bin tool properties
include(${ZEPHYR_BASE}/cmake/bintools/gnu/target_bintools.cmake)
