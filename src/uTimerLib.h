/**
 * \mainpage
 * \brief Arduino tiny and cross-device compatible timer library.
 *
 * Timers used by each microcontroller:
 *		* Atmel ATtiny:		Timer1 (2nd timer) - https://github.com/damellis/attiny and https://github.com/SpenceKonde/Disgispark AVRCore (25, 45 and 85)
 *		* DisgiSpark AVR:	Timer0 (1st timer) - https://github.com/digistump/DigistumpArduino
 *		* Atmel AVR 32U4:	Timer3 (4rd timer)
 *		* Atmel AVR other:	Timer2 (3rd timer)
 *		* STM32:			Timer3 (3rd timer)
 *		* SAM (Due):		TC3 (Timer1, channel 0)
 *		* ESP8266:			OS Timer, one slot of seven available (Software timer provided by Arduino because ESP8266 has only two hardware timers and one is needed by it normal operation)
 *		* ESP32:			OS Hardware Timer.
 *		* SAMD21:			Timer 4, CC0 (TC3). See http://ww1.microchip.com/downloads/en/DeviceDoc/40001882A.pdf
 *		* SAMD51:			Timer 2 (TC1), 16 bits mode (See http://ww1.microchip.com/downloads/en/DeviceDoc/60001507C.pdf
 *
 * You have public TimerLib variable with following methods:
 *		* TimerLib.setInterval_us(callback_function, microseconds);* : callback_function will be called each microseconds.
 *		* TimerLib.setInterval_s(callback_function, seconds);* : callback_function will be called each seconds.
 *		* TimerLib.setTimeout_us(callback_function, microseconds);* : callback_function will be called once when microseconds have passed.
 *		* TimerLib.setTimeout_s(callback_function, seconds);* : callback_function will be called once when seconds have passed.
 *		* TimerLib.clearTimer();* : will clear any timed function if exists.
 *
 * @file hardware/uTimerLib.ATTINY.cpp
 * @copyright Naguissa
 * @author Naguissa
 * @see <a href="https://github.com/Naguissa/uTimerLib">https://github.com/Naguissa/uTimerLib</a>
 * @see <a href="https://www.foroelectro.net/librerias-arduino-ide-f29/utimerlib-libreria-arduino-para-eventos-temporizad-t191.html">https://www.foroelectro.net/librerias-arduino-ide-f29/utimerlib-libreria-arduino-para-eventos-temporizad-t191.html</a>
 * @see <a href="mailto:naguissa@foroelectro.net">naguissa@foroelectro.net</a>
 * @version 1.7.5
 */
/** \file uTimerLib.h
 *   \brief uTimerLib header file
 */
#ifndef _uTimerLib_
	/**
	 * \brief Prevent multiple inclussion
	 */
	#define _uTimerLib_

	#include "Arduino.h"

	#if defined(ARDUINO_ARCH_ESP8266)
		#include <Ticker.h>  //Ticker Library
	#endif

	#if defined(ARDUINO_ARCH_ESP32)
		#include "esp_timer.h"
	#endif

	// Operation modes
	/**
	 * \brief Internal status
	 */
	#define UTIMERLIB_TYPE_OFF 0
	/**
	 * \brief Internal status
	 */
	#define UTIMERLIB_TYPE_TIMEOUT 1
	/**
	 * \brief Internal status
	 */
	#define UTIMERLIB_TYPE_INTERVAL 2

	#if defined(_VARIANT_ARDUINO_STM32_) || defined(ARDUINO_ARCH_STM32)
		#include "HardwareTimer.h"

		// ST's Arduino Core STM32, https://github.com/stm32duino/Arduino_Core_STM32
		#ifdef BOARD_NAME
			// Private member

		// Roger Clark Arduino STM32, https://github.com/rogerclarkmelbourne/Arduino_STM32
		#else
			extern HardwareTimer Timer3;
		#endif
	#endif

	class uTimerLib {
		public:
			uTimerLib();
			void setInterval_us(void (*) (), unsigned long int);
			void setInterval_s(void (*) (), unsigned long int);
			void setTimeout_us(void (*) (), unsigned long int);
			void setTimeout_s(void (*) (), unsigned long int);

			/**
			 * \brief Loads last bit of time needed to precisely count until desired time (non complete loop)
			 *
			 * Note: This is device-dependant
			 */
			void clearTimer();


			#if defined(_VARIANT_ARDUINO_STM32_) || defined(ARDUINO_ARCH_STM32)
			    volatile static unsigned long int _overflows;
			    volatile static unsigned long int __overflows;
			    volatile static unsigned long int _remaining;
			    volatile static unsigned long int __remaining;
			    static void (*_cb)();
				static void _interrupt();
    			volatile static unsigned char _type;
			#else
			    /**
			     * \brief Internal intermediate function to control timer interrupts
			     *
			     * As timers doesn't give us enougth flexibility for large timings,
			     * this function implements oferflow control to offer user desired timings.
			     */
			    void _interrupt();					
			#endif
					
			#if defined(ARDUINO_ARCH_ESP32)
				static void interrupt(void* arg);
			#endif

			#if defined(ARDUINO_ARCH_ESP8266)
				#pragma message "ESP8266 can only reach a ms resolution so any us interrupt will be rounded to that"
				static void interrupt();
			#endif

			#ifdef _SAMD21_
				TcCount16* _TC = (TcCount16*) TC3;
			#endif

			#ifdef __SAMD51__
				#pragma message "SAMD51 support is still experimental"
			#endif

		private:
			static uTimerLib *_instance;

			#if defined(_VARIANT_ARDUINO_STM32_) || defined(ARDUINO_ARCH_STM32)
				bool _toInit = true;

				// ST's Arduino Core STM32, https://github.com/stm32duino/Arduino_Core_STM32
				#ifdef BOARD_NAME
					 HardwareTimer *Timer3 = new HardwareTimer(TIM3);
                #endif
			#else
			    volatile unsigned long int _overflows = 0;
			    volatile unsigned long int __overflows = 0;
			    #ifdef ARDUINO_ARCH_AVR
				    volatile unsigned char _remaining = 0;
				    volatile unsigned char __remaining = 0;
			    #else
				    volatile unsigned long int _remaining = 0;
				    volatile unsigned long int __remaining = 0;
			    #endif
			    void (*_cb)() = NULL;
			    volatile unsigned char _type = UTIMERLIB_TYPE_OFF;
			#endif

			void _loadRemaining();

			void _attachInterrupt_us(unsigned long int);
			void _attachInterrupt_s(unsigned long int);


			#if defined(ARDUINO_ARCH_ESP32)
				esp_timer_handle_t _timer;
			#endif

			#if defined(ARDUINO_ARCH_ESP8266)
				Ticker _ticker;
			#endif

	};

	/**
	 * \brief Declares TimerLib variable to access this library's methods
	 */
	extern uTimerLib TimerLib;

#endif

