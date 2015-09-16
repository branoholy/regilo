NeatoC
======

*A simple C++ library for communication with the Neato XV robot.*

`neatoc` allows you to communicate with the Neato robot through sockets. You 
can use some build-in methods like `setMotor()`, `getLdsScan()`, or run any 
other command with `sendCommand()` method.

Download
--------
You can download the source code and build `neatoc` according to 
the [build instructions](#build) below.

Usage
-----
```c++
// Create the controller
neatoc::Controller controller;

// Connect it
controller.connect("10.0.0.1:12345");

// Set test mode and LDS rotation
controller.setTestMode(true);
controller.setLdsRotation(true);

// Grab a scan from the robot
neatoc::ScanData data = controller.getLdsScan();

// Unset test mode and LDS rotation
controller.setLdsRotation(false);
controller.setTestMode(false);
```

Dependencies
------------
The library uses:

* [Boost.Asio](http://www.boost.org/doc/libs/release/doc/html/boost_asio.html) 
library (version 1.59 or newer). It is necessary to link the `boost_system` 
library.
* [Boost String Algorithms Library](http://www.boost.org/doc/libs/release/doc/html/string_algo.html) 
(version 1.59 or newer).

The `neatoc-scan-gui` example also needs:

* [wxWidgets](https://www.wxwidgets.org) library (version 3.0 or newer).

Build
-----
Make sure you have installed all [dependencies](#dependencies) before building.

```bash
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

For faster build on a multicore processor, you can use:

```bash
$ cmake --build . -- -j4
```

Where the option `-j4` means you want to use 4 cores for building.

License
-------
NeatoC is licensed under GNU GPL v3 (see 
[LICENSE](https://github.com/branoholy/neatoc/blob/master/LICENSE) 
file).


