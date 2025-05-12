# clapkit

## Introduction

Clapkit (Classic Application Kit) is basically a C++/object-oriented wrapper for Macintosh Toolbox functions. 
The goal is to eventually add Carbon/Cocoa and even Win32 support to allow cross-platform development.

## Installation

To 'import' Clapkit, you can use FetchContent:

```CMake
include(FetchContent)
FetchContent_Declare(
    clapkit
    GIT_REPOSITORY https://github.com/macinlink/clapkit.git
    GIT_TAG main
)
FetchContent_MakeAvailable(clapkit)
target_link_libraries(my_project PRIVATE clapkit)
```

You may need CMakeLists.txt and .vscode/c_cpp_properties.json modified to point to your Retro68 installation.

## Usage

Create an object for your app by subclassing CApp.

```C++
class MyNewApp: public CKApp {
    public:
        MyNewApp() { }
        ~MyNewApp() { }
};
```

Then 'start' your app:

```C++
app = CKNew RetroScript();
```

You'll need to write a run-loop as well:

```C++
while(1) {
    int result = app->Loop(5);
    if (result != 0) {
        // Exit the loop, done here.
        break;
    }
}
```