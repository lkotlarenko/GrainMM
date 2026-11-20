# SPDX-License-Identifier: GPL-3.0-only
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
if(NOT DEFINED ENV{GRAINMM_SDK})
  message(FATAL_ERROR "Set GRAINMM_SDK to the extracted miyoomini-toolchain directory")
endif()
file(REAL_PATH "$ENV{GRAINMM_SDK}" SDK)
set(CMAKE_CXX_COMPILER "${SDK}/bin/arm-linux-gnueabihf-g++")
set(CMAKE_C_COMPILER "${SDK}/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_SYSROOT "${SDK}/arm-linux-gnueabihf/libc")
set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}")
set(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "")
# Conservative VFP path; do not assume NEON from the CPU family alone.
set(CMAKE_CXX_FLAGS_INIT "-mcpu=cortex-a7 -mfpu=vfpv3-d16 -mfloat-abi=hard")
# Release optimization is set explicitly by the preset; *_INIT is augmented
# by CMake's compiler defaults and would otherwise end with an overriding -O3.
set(GRAINMM_SDL1 ON CACHE BOOL "Miyoo SDK SDL 1.2" FORCE)
