# Kitchen Sink (fka CKTest)

A simple example app for [Clapkit](https://github.com/macinlink/clapkit) - shows how to use most of the API, including but not limited to:

* Networking
* UI Elements (menus, buttons, checkboxes, etc.)
* App Life-cycle
* Setting up the required resources (see `cktest.r`)

![Screenshot](https://raw.githubusercontent.com/macinlink/clapkit-test/refs/heads/main/Clapkit-Test-App.png)

It is tested on vMac and real hardware running Mac OS 7.6 and 9.1.

## Building

### Create a build folder

Make a build folder, switch to it.
```
mkdir build
cd build
```

### Parameters of Interest

* `CMAKE_BUILD_TYPE` can be `Debug` or `Release`
  * You'll need to have Macsbug installed for debug mode, or the application will crash on start. Debug mode also keeps track of memory usage and leaks.
  * Clapkit is very chatty in the debugs - you might want to run `dx off` on Macsbug or else you'll be entering "g + ENTER" a lot.
* `USE_LOCAL_CLAPKIT` - if `OFF`, will fetch Clapkit from Github. Else, will use local copy.
  * This is hard-coded to `${CMAKE_SOURCE_DIR}/../../` for now.

### Build it

Example:
```
cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_LOCAL_CLAPKIT=ON ..
```

### Run it

Then finally, make and run:
```
make && LaunchAPPL -e minivmac CKTest.APPL
```

If you have a Mac running [LaunchAPPL](https://github.com/autc04/Retro68/tree/master/LaunchAPPL/Client), you can also do something like:

```
make && LaunchAPPL -e tcp ./CKTest.bin --tcp-address 192.168.1.xxx
```

### Play with it

Select an option from the `Tests` menu to try UI elements and patterns like:

* Buttons, checkboxes, radioboxes, etc.
* How to show message boxes
* How to setup timers
* How to use the MacTCP wrapper