/**
 * Tiny and cross-device compatible timer library.
 *
 * Timers used by microcontroller
 *	Atmel AVR: Timer2
 *
 * @copyright Naguissa
 * @author Naguissa
 * @email naguissa@foroelectro.net
 * @version 0.1.0
 * @created 2018-01-27
 */
#ifndef _uTimerLib_
	#define _uTimerLib_

	#include "Arduino.h"

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

			#ifdef _VARIANT_ARDUINO_STM32_
				static void interrupt();
			#endif
			void _interrupt();


		private:
			/**
			 * Because several compatibility issues -specially STM32- we need to put
			 * these as public, but it should be private. Maybe in future upgrades...
			 */
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

	};

	extern uTimerLib TimerLib;

#endif

