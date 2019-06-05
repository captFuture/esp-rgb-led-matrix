# esp-rgb-led-matrix
Full RGB LED matrix, based on an ESP32 and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

## Motivation
Having a remote display to show any kind of information, running 24/7 reliable.

## Requirements
See [requirements](https://github.com/BlueAndi/esp-rgb-led-matrix/blob/master/doc/REQUIREMENTS.md).

## Electronic
The main parts are:
* [NodeMCU development board ESP-32S](https://docs.zerynth.com/latest/official/board.zerynth.nodemcu_esp32/docs/index.html)
* WS2812B 5050 8x32 RGB Flexible LED Matrix Panel
* Power supply 5 V / 4 A

See [electronic detail](https://github.com/BlueAndi/esp-rgb-led-matrix/blob/master/doc/ELECTRONIC.md).

## Software

### IDE
The [PlatformIO IDE](https://platformio.org/platformio-ide) is used for the development. Its build on top of Microsoft Visual Studio Code.

### Installation
1. Install [VSCode](https://code.visualstudio.com/).
2. Install PlatformIO IDE according to this [HowTo](https://platformio.org/install/ide?install=vscode).
3. Close and start VSCode again.
4. Recommended is to take a look to the [quick-start guide](https://docs.platformio.org/en/latest/ide/vscode.html#quick-start).

### Compiling project
1. Load workspace in VSCode.
2. Change to PlatformIO toolbar.
3. _Project Tasks -> Build_ or via hotkey ctrl-alt-b

### Used libraries
* [Arduino](https://docs.platformio.org/en/latest/frameworks/arduino.html#framework-arduino) - ESP framework
* [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) - Controlling the LED matrix with hardware support (I2S).

### Structure

<pre>
+---doc             (Documentation)
    +---datasheets  (Datasheets of electronic parts)
    \---design      (Desgin related documents)
+---include         (Include files)
+---lib             (Project specific (private) libraries)
+---src             (Sourcecode)
+---test            (Unit tests)
\---platform.ini    (Project configuration file)
</pre>

## Issues, Ideas and bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

## License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).