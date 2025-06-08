cmake_minimum_required(VERSION 3.16)

# Ensure Retro68 toolchain is set BEFORE defining project
set(RETRO68_PATH "$ENV{HOME}/Retro68-build/toolchain/m68k-apple-macos")
set(CMAKE_TOOLCHAIN_FILE "${RETRO68_PATH}/cmake/retro68.toolchain.cmake")

project(clapkit VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include Retro68 headers
include_directories(${RETRO68_PATH}/include)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug or Release)" FORCE)
endif()

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Source Dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "ClapKit Dir: (if using local) ${CMAKE_SOURCE_DIR}/../../")


set(CLAPKIT_LOCAL_PATH "${CMAKE_SOURCE_DIR}/../../")
option(USE_LOCAL_CLAPKIT "Use local checkout of clapkit instead of fetching from Git" ON)

if(USE_LOCAL_CLAPKIT AND EXISTS "${CLAPKIT_LOCAL_PATH}/CMakeLists.txt")
    message(STATUS "Using local clapkit at ${CLAPKIT_LOCAL_PATH}")
    add_subdirectory(${CLAPKIT_LOCAL_PATH} clapkit)
    set(USE_LOCAL_CLAPKIT ON)
else()
    message(STATUS "Fetching clapkit from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        clapkit
        GIT_REPOSITORY https://github.com/macinlink/clapkit.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(clapkit)
    set(USE_LOCAL_CLAPKIT OFF)
endif()

# Add application
add_application(CKTest
    TYPE "APPL"
    CREATOR "CKTS"

    main.cpp
    cktest.r
)

# Debug vs Release settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug Build")
    target_compile_definitions(CKTest PRIVATE kCKAPPDEBUG=1)
    target_compile_definitions(clapkit PRIVATE kCKAPPDEBUG=1)

    set_target_properties(CKTest PROPERTIES 
        COMPILE_FLAGS "-O0 -Wall -g -fdata-sections -ffunction-sections"
        LINK_FLAGS "-Wl,--mac-single"
    )

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring Release Build")
    target_compile_definitions(CKTest PRIVATE NDEBUG)  # Disable debugging in Release
    target_compile_definitions(clapkit PRIVATE NDEBUG)

    set_target_properties(CKTest PROPERTIES 
        COMPILE_FLAGS "-s -O1 -Wall -fmerge-all-constants -fmerge-constants -fstrict-aliasing -fomit-frame-pointer -fdata-sections -ffunction-sections -finline-functions"
        LINK_FLAGS "-Wl,--mac-single -Wl,--mac-strip-macsbug"
    )
endif()

# Link clapkit
target_link_libraries(CKTest PRIVATE clapkit)