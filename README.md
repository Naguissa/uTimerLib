# Arduino tiny and cross-device compatible timer library - uTimerLib #

## What is this repository for? ##

Tiny and cross-device compatible timer-driven timed function calls library.

Supports Arduino AVR, SAM, STM32, ESP8266, ESP32 and SAMD21 microcontrollers.


## Current status ##

Currently suported architectures:
 - AVR
 - STM32
 - SAM (Arduino Due)
 - ESP8266
 - ESP32
 - SAMD21 (Arduino Zero and Zero MKR; experimental support)

 - SAMD51 (Adafruit Feather M4, Adafruit Metro M4; work in progress, not functional -need for a SAMD51 board...)

## Device timer usage ##

Depending on wich architecture this library uses one or another device timer. Take in mind this because can caause conflicts with other libraries:

 - AVR: Timer2 (3rd timer)
 - STM32: Timer3 (3rd timer)
 - SAM: TC3 (Timer1, channel 0)
 - ESP8266: Ticker library (inside ESP8266 core, no extras needed)
 - ESP32: OS Timer, one slof of software timer.
 - SAMD21: Timer3 (4th timer), CC0; 16 bit mode
 - SAMD51: Timer1 (2nd timer); 16 bit mode

*Note*: On ESP8266 and ESP32 this library uses "ticker" to manage timer, so it's maximum resolution is miliseconds. On "_us" functions times will be rounded to miliseconds.

## Usage ##

This library defines a global variable when included called "TimerLib".

You have these methods:
 - *TimerLib.setInterval_us(callback_function, microseconds);* : callback_function will be called each microseconds.
 - *TimerLib.setInterval_s(callback_function, seconds);* : callback_function will be called each seconds.
 - *TimerLib.setTimeout_us(callback_function, microseconds);* : callback_function will be called once when microseconds have passed.
 - *TimerLib.setTimeout_s(callback_function, seconds);* : callback_function will be called once when seconds have passed.
 - *TimerLib.clearTimer();* : will clear any timed function if exists.

It only manages one function at a time, if you call any setXXX method it will cancel any running timed function and process new one.

An attached functions broker could be implemented, but then this would not be (micro)TimerLib. Maybe in other project.....

## How do I get set up? ##

You can get it from Arduino libraries directly, searching by uTimerLib.

For manual installation:

 * Get the ZIP from releases link: https://github.com/Naguissa/uTimerLib/releases
 * Rename to uTimerLib.zip
 * Install library on Arduino

## Examples ##

Included on example folder, available on Arduino IDE.


## Extra ##

Look in extras folder for datasheets and extra info


## Who do I talk to? ##

 * [Naguissa](https://github.com/Naguissa)
 * https://www.foroelectro.net/electronica-digital-microcontroladores-f8/utimerlib-libreria-arduino-para-eventos-temporizad-t191.html
 * https://www.naguissa.com


## Contribute ##

Any code contribution, report or comment are always welcome. Don't hesitate to use GitHub for that.


You can make a donation via PayPal: https://paypal.me/foroelectro


Thanks for your support.
