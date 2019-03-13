/**
 * Arduino tiny and cross-device compatible timer library.
 *
 * Timers used by microcontroller
 *	Atmel AVR:	Timer2 (3rd timer)
 *  STM32:		Timer3 (3rd timer)
 *  SAM (Due):  TC3 (Timer1, channel 0)
 *  ESP8266:	OS Timer, one slof of seven available (Software timer provided by Arduino because ESP8266 has only two hardware timers and one is needed by it normal operation)
 *  ESP32:		OS Timer, one slof of software timer.
 *  SAMD21:		Timer 4, CC0 (TC3). See http://ww1.microchip.com/downloads/en/DeviceDoc/40001882A.pdf
 *  SAMD51:		Timer 2 (TC1), 16 bits mode (See http://ww1.microchip.com/downloads/en/DeviceDoc/60001507C.pdf
 *
 * @copyright Naguissa
 * @author Naguissa
 * @email naguissa@foroelectro.net
 * @version 1.1.2
 * @created 2018-01-27
 */
#ifndef _uTimerLib_
	#define _uTimerLib_

	#include "Arduino.h"

	#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
		#include <Ticker.h>  //Ticker Library
	#endif
	// Operation modes
	#define UTIMERLIB_TYPE_OFF 0
	#define UTIMERLIB_TYPE_TIMEOUT 1
	#define UTIMERLIB_TYPE_INTERVAL 2

	#ifdef _VARIANT_ARDUINO_STM32_
		#include "HardwareTimer.h"
		extern HardwareTimer Timer3;
	#endif

	class uTimerLib {
		public:
			uTimerLib();
			void setInterval_us(void (*) (), unsigned long int);
			void setInterval_s(void (*) (), unsigned long int);
			int setTimeout_us(void (*) (), unsigned long int);
			int setTimeout_s(void (*) (), unsigned long int);
			void clearTimer();
			void _interrupt();

			#ifdef _VARIANT_ARDUINO_STM32_
				static void interrupt();
			#endif

			#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
				#pragma message "ESP8266 / ESP32 can only reach a ms resolution so any ms interrupt will be rounded to that"
				static void interrupt();
			#endif

			#ifdef _SAMD21_
				#pragma message "SAMD21 support is still experimental"
				TcCount16* _TC = (TcCount16*) TC3;
			#endif

			#ifdef __SAMD51__
				#pragma message "SAMD51 support is still experimental"
			#endif

		private:
			static uTimerLib *_instance;

			unsigned long int _overflows = 0;
			unsigned long int __overflows = 0;
			#ifdef ARDUINO_ARCH_AVR
				unsigned char _remaining = 0;
				unsigned char __remaining = 0;
			#else
				unsigned long int _remaining = 0;
				unsigned long int __remaining = 0;
			#endif
			void (*_cb)() = NULL;
			unsigned char _type = UTIMERLIB_TYPE_OFF;

			void _loadRemaining();

			void _attachInterrupt_us(unsigned long int);
			void _attachInterrupt_s(unsigned long int);

			#ifdef _VARIANT_ARDUINO_STM32_
				bool _toInit = true;
			#endif

			#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
				Ticker _ticker;
			#endif
	};

	extern uTimerLib TimerLib;

#endif

