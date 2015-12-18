# NeatoC
[![Build Status](https://travis-ci.org/branoholy/neatoc.svg?branch=master)](https://travis-ci.org/branoholy/neatoc)

*A simple C++ library for communication with the Neato XV robot.*

`neatoc` allows you to communicate with the Neato robot through sockets. You
can use some implemented methods like `setMotor()`, `getLdsScan()`, or run any
other command with the `sendCommand()` method.

## Download
You can download the source code and build `neatoc` according to
the [build instructions](#build) below.

## Usage
```cpp
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

## Dependencies
The library uses:

* [Boost.Asio](http://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
library (version 1.54 or newer). It is necessary to link the `boost_system`
library.
* [Boost String Algorithms Library](http://www.boost.org/doc/libs/release/doc/html/string_algo.html)
(version 1.54 or newer).

The `neatoc-scan-gui` example also needs:

* [wxWidgets](https://www.wxwidgets.org) library (version 3.0 or newer).

## Build
Make sure you have installed all [dependencies](#dependencies) before building.

```text
$ mkdir build && cd build
$ cmake ..
$ make
```

Use the following option if you want to build the [examples](https://github.com/branoholy/neatoc/tree/master/examples)
as well:

```text
$ cmake -Dexamples:bool=on ..
```

For faster build on a multicore processor, you can use:

```text
$ make -j$(nproc)
```

## Installation
To install the `neatoc` library (and its examples), simply run as root:

```text
# make install
```

To uninstall:

```text
# make uninstall
```

### Packages
You can also install `neatoc` in Arch Linux from the [AUR](https://aur.archlinux.org/packages/neatoc).

Do not forget to add my PGP key ([FBAC61E6](http://pgp.mit.edu/pks/lookup?search=0xFBAC61E6&fingerprint=on),
fingerprint `D258 09BF 3563 AA56 A12B  0F4D 545E DD46 FBAC 61E6`).

```text
$ gpg --recv-key FBAC61E6
```

## Examples
See [examples](https://github.com/branoholy/neatoc/tree/master/examples) for
more information about using of this library.

`neatoc-scan` is a simple example that connects to the Neato and performs one
scan that is printed.

`neatoc-scan-gui` is more complex and requires wxWidgets library. It can be used
 to drive with the Neato, scan automatically or manually, and log the output.

## License
NeatoC is licensed under GNU GPL v3 (see
[LICENSE](https://github.com/branoholy/neatoc/blob/master/LICENSE)
file).

