# Arduino, ESP8266 and STM32 Timer-driven timed function calls - uTimerLib #

## What is this repository for? ##

Tiny and cross-device compatible timer-driven timed function calls library.

Supports Arduino AVR, SAM, SAM32, STM32 and ESP8266 microcontrollers.


## Current status ##

While being in 0.X.Y version, library is still in development.

Currently suported architectures:
 - AVR
 - STM32

## Device timer usage ##

Depending on wich architecture this library uses one or another device timer. Take in mind this because can caause conflicts with other libraries:

 - AVR: Timer2 (3rd timer)
 - STM32: Timer3 (3rd timer)

## Usage ##

This library defines a global avriable when included called "TimerLib".

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



## Who do I talk to? ##

 * [Naguissa](https://github.com/Naguissa)
 * https://www.foroelectro.net/electronica-digital-microcontroladores-f8/utimerlib-libreria-arduino-para-eventos-temporizad-t191.html
 * https://www.naguissa.com

