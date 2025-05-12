cmake_minimum_required(VERSION 3.24)

# Ensure Retro68 toolchain is set BEFORE defining project
set(CMAKE_TOOLCHAIN_FILE "~/Retro68-build/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake")

project(clapkit VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Clapkit: Configuring Debug Build")
    target_compile_definitions(clapkit PRIVATE kCKAPPDEBUG=1)
    target_compile_definitions(clapkit PRIVATE kCKDEBUGMEMORY=1)

    set_target_properties(clapkit PROPERTIES 
        COMPILE_FLAGS "-O0 -Wall -g -fdata-sections -ffunction-sections"
        LINK_FLAGS "-Wl,--mac-single"
    )

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Clapkit: Configuring Release Build")
    target_compile_definitions(clapkit PRIVATE NDEBUG)

    set_target_properties(clapkit PROPERTIES 
        COMPILE_FLAGS "-s -O1 -Wall -fmerge-all-constants -fmerge-constants -fstrict-aliasing -fomit-frame-pointer -fdata-sections -ffunction-sections -finline-functions"
        LINK_FLAGS "-Wl,--mac-single -Wl,--mac-strip-macsbug"
    )
endif()

target_include_directories(clapkit PUBLIC include)