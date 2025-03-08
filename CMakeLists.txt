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
    src/ckApp.cpp
    src/ck_pFocusableControl.cpp
    src/ck_pTextableControl.cpp
    src/ckObject.cpp
    src/ckWindow.cpp
    src/ckUtils.cpp
    src/ckControl.cpp
    src/ckControlToolbox.cpp
    src/ckButton.cpp
    src/ckLabel.cpp
    src/ckCheckbox.cpp
    src/ckCanvas.cpp
    src/ckTextField.cpp
)

# Enable -ffunction-sections and -gc-sections to make the app as small as possible
# On 68K, also enable --mac-single to build it as a single-segment app (so that this code path doesn't rot)
set_target_properties(clapkit PROPERTIES COMPILE_OPTIONS -ffunction-sections)
if(CMAKE_SYSTEM_NAME MATCHES Retro68)
    set_target_properties(clapkit PROPERTIES LINK_FLAGS "-Wl,-gc-sections -Wl,--mac-single")
else()
    set_target_properties(clapkit PROPERTIES LINK_FLAGS "-Wl,-gc-sections")
endif()

target_include_directories(clapkit PUBLIC include)