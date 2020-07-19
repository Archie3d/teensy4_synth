# Experimenting with audio synthesis on Teensy 4.0 board

## Introduction
This is an experimental code with no particular goal. It does not use Teensy audio library (except for the feeding `AudioStream` object).
The code assumes an I2S audio interface connected to the Teensy4.0 board. I use UDA1334 break-out board from Adafuit (https://www.adafruit.com/product/3678) 

### Pinout
Check the board info and pinout diagram (https://www.pjrc.com/store/teensy40.html)

| Teensy 4.0 | UDA1334 |
|:-----------|:--------|
| pin 7      | DIN     |
| pin 20     | WSEL    |
| pin 21     | BCLK    |
| 3V         | VIN     |
| GND        | GND     |


## Compile
1. Make sure to have Arduino adn Teensyduino add-on installed (see https://www.pjrc.com/teensy/td_download.html)
2. Copy build tools to `./tools` folder
3. You'll need to have a `make` tool to compile the code. On Windows you may use `Msys` package (https://www.msys2.org/).
4. Connect Teensy4 board via USB and run:
```shell
$ cd src
$ make
```
This will build the code and program the board. No need to run Arduino IDE.

## Playing
Currently the code enabled USB MIDI and Serial interfaces. When board is connected via USB cable you should see `Teensy MIDI` interface where you can send MIDI commands to. USB virtual serial port is used to print debug info.

## Example output
Recorded directly fromt eh audio output.

https://soundcloud.com/arthur-benilov/moszkowski-etincelles-op-36-no-6
