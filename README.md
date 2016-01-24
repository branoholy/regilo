# Regilo
[![Build Status](https://travis-ci.org/branoholy/regilo.svg?branch=master)](https://travis-ci.org/branoholy/regilo)

*A simple C++ library for controlling the Neato XV robot and the Hokuyo scanner.*

`regilo` allows you to communicate with the Neato robot through sockets or the
Hokuyo scanner through the serial port. You can use some implemented methods
like `setTestMode()`, `setMotor()` (for Neato XV), `getVersion()` (for Hokuyo),
`getScan()` for both, or run any other command with the `sendCommand()` method.

## Download
You can download the source code and build `regilo` according to
the [build instructions](#build) below.

## Usage

### Neato XV
```cpp
// Create the controller
regilo::NeatoController controller;

// Connect it
controller.connect("10.0.0.1:12345");

// Set test mode and LDS rotation
controller.setTestMode(true);
controller.setLdsRotation(true);

// Grab a scan from the robot
regilo::ScanData data = controller.getScan();

// Unset test mode and LDS rotation
controller.setLdsRotation(false);
controller.setTestMode(false);
```

### Hokuyo
```cpp
// Create the controller
regilo::HokuyoController controller;

// Connect it
controller.connect("/dev/ttyACM0");

// Grab a scan from the scanner
regilo::ScanData data = controller.getScan();
```

## Dependencies
The library uses

* [Boost.Asio](http://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
library (version 1.54 or newer),
* [Boost String Algorithms Library](http://www.boost.org/doc/libs/release/doc/html/string_algo.html)
(version 1.54 or newer).

The `regilo-visual` example also needs

* [wxWidgets](https://www.wxwidgets.org) library (version 3.0 or newer).

## Build
Make sure you have installed all [dependencies](#dependencies) before building.

```text
$ mkdir build && cd build
$ cmake ..
$ make
```

Use one of the following options if you want to build the [examples](https://github.com/branoholy/regilo/tree/master/examples)
as well.

* `$ cmake -Dexample:bool=on ..` for the console example (`regilo-scan`).
* `$ cmake -Dexample-gui:bool=on ..` for the GUI example (`regilo-visual`).
* `$ cmake -Dexamples:bool=on ..` for all examples.

For a faster build on a multicore processor, you can use:

```text
$ make -j$(nproc)
```

## Installation
To install the `regilo` library (and its examples), simply run as root:

```text
# make install
```

To uninstall:

```text
# make uninstall
```

### Packages

#### Arch Linux
You can install `regilo` in Arch Linux from the [AUR](https://aur.archlinux.org/packages/regilo).

Do not forget to add [my PGP key](http://pgp.mit.edu/pks/lookup?search=0xD25809BF3563AA56A12B0F4D545EDD46FBAC61E6&fingerprint=on)
(fingerprint `D258 09BF 3563 AA56 A12B  0F4D 545E DD46 FBAC 61E6`).

```text
$ gpg --recv-key D25809BF3563AA56A12B0F4D545EDD46FBAC61E6
```

#### Ubuntu
In Ubuntu, you can use my [ppa:branoholy/regilo](https://launchpad.net/~branoholy/+archive/ubuntu/regilo)
and install the `libregilo-dev` package.

```text
sudo add-apt-repository ppa:branoholy/regilo
sudo apt-get update
sudo apt-get install libregilo-dev
```

## Examples
See [examples](https://github.com/branoholy/regilo/tree/master/examples) for
more information about using of this library.

`regilo-scan` is a simple example that connects to the Neato or Hokuyo and
performs one scan that is printed.

`regilo-visual` is more complex and requires wxWidgets library. It can be used
to drive with the Neato, scan automatically or manually, and log the output.
Same scanning functionality can be done with the Hokuyo as well.

## License
Regilo is licensed under GNU GPL v3 (see
[LICENSE](https://github.com/branoholy/regilo/blob/master/LICENSE)
file).

