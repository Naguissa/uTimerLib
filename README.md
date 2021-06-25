# Arduino tiny and cross-device compatible timer library - uTimerLib #

Master status:   [![Build Status](https://travis-ci.org/Naguissa/uTimerLib.svg?branch=master)](https://travis-ci.org/Naguissa/uTimerLib)

## What is this repository for? ##

Tiny and cross-device compatible timer-driven timed function calls library.

Supports Arduino AVR, SAM, STM32, ESP8266, ESP32 and SAMD21 microcontrollers.


## Current status ##

Currently suported architectures:
 - Atmel ATtiny, Experimental - https://github.com/damellis/attiny and https://github.com/SpenceKonde/Disgispark AVRCore (25, 45 and 85)
 - DisgiSpark AVR, Experimental - https://github.com/digistump/DigistumpArduino
 - Atmel AVR 32U4
 - Atmel AVR, general
 - STM32 (both Roger Clark and ST cores)
 - SAM (Due)
 - ESP8266
 - ESP32
 - SAMD21 (Arduino Zero and Zero MKR; experimental support)
 - SAMD51 (Adafruit Feather M4, Adafruit Metro M4; work in progress, not functional -need for a SAMD51 board...)

## Device timer usage ##

Depending on wich architecture this library uses one or another device timer. Take in mind this because can caause conflicts with other libraries:
 - Atmel ATtiny:	Timer1 (2nd timer).
 - DisgiSpark AVR:	Timer0 (1st timer).
 - Atmel AVR 32U4:	Timer3 (4rd timer).
 - Atmel AVR other:	Timer2 (3rd timer).
 - STM32:			Timer3 (3rd timer).
 - SAM (Due):		TC3 (Timer1, channel 0).
 - ESP8266:			OS Timer, one slot of seven available (Software timer provided by Arduino because ESP8266 has only two hardware timers and one is needed by it normal operation).
 - ESP32:			OS Hardware Timer.
 - SAMD21:			Timer 4, CC0 (TC3).
 - SAMD51:			Timer 2 (TC1), 16 bits mode.

*Note*: On ESP8266 this library uses "ticker" to manage timer, so it's maximum resolution is miliseconds. On "_us" functions times will be rounded to miliseconds.

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

## Documentation and extras ##

You can find all documentation and extras in this repository: https://github.com/Naguissa/uTimerLib_doc_and_extras

You can read documentation online here: https://naguissa.github.io/uTimerLib_doc_and_extras/



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


 * You can sponsor this project using GitHub's Sponsor button: https://github.com/Naguissa/uTimerLib
 * You can make a donation via PayPal: https://paypal.me/foroelectro


Thanks for your support.


Contributors hall of fame: https://www.foroelectro.net/hall-of-fame-f32/contributors-contribuyentes-t271.html
