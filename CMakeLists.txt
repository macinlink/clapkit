cmake_minimum_required(VERSION 3.24)

# Ensure Retro68 toolchain is set BEFORE defining project
set(CMAKE_TOOLCHAIN_FILE "~/Retro68-build/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake")

project(clapkit VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define Retro68 path properly
set(RETRO68_PATH ~/Retro68-build/toolchain/m68k-apple-macos)

# Include Retro68 headers
include_directories(${RETRO68_PATH}/include)

# Add library
add_library(clapkit
    src/dlmalloc.c
    src/dlmalloc_mac.c
    src/ckMemory.cpp
    src/ckUtils.cpp
    src/ckApp.cpp
    src/ck_pFocusableControl.cpp
    src/ck_pTextableControl.cpp
    src/ckObject.cpp
    src/ckWindow.cpp
    src/ckControl.cpp
    src/ckControlToolbox.cpp
    src/ckButton.cpp
    src/ckLabel.cpp
    src/ckCheckbox.cpp
    src/ckCanvas.cpp
    src/ckTextField.cpp
    src/ckTimer.cpp
)

target_include_directories(clapkit
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(clapkit
  PRIVATE
    $<$<CONFIG:Debug>:kCKAPPDEBUG=1;kCKDEBUGMEMORY=1>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(clapkit
  PRIVATE
    $<$<CONFIG:Debug>:-O0;-g;-Wall;-fdata-sections;-ffunction-sections>
    $<$<CONFIG:Release>:-s;-O1;-Wall;-fmerge-all-constants;-fmerge-constants;-fstrict-aliasing;-fomit-frame-pointer;-fdata-sections;-ffunction-sections;-finline-functions>
)

target_link_options(clapkit
  PRIVATE
    $<$<CONFIG:Debug>:-Wl,--mac-single>
    $<$<CONFIG:Release>:-Wl,--mac-single;-Wl,--mac-strip-macsbug>
)