/**
 * \class uTimerLib
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
 *		* ESP32:			OS Timer, one slot of software timer.
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
 * @version 1.6.4
 */

#if (defined(__AVR_ATmega32U4__) || defined(ARDUINO_ARCH_AVR)) && !defined(ARDUINO_attiny) && defined(UTIMERLIB_HW_COMPILE)
#if	!defined(_uTimerLib_IMP_) && defined(_uTimerLib_cpp_)
	#define _uTimerLib_IMP_
	#include "uTimerLib.cpp"


	/**
	 * \brief Sets up the timer, calculation variables and interrupts for desired ms microseconds
	 *
	 * Note: This is device-dependant
	 *
	 * @param	us		Desired timing in microseconds
	 */
	void uTimerLib::_attachInterrupt_us(unsigned long int us) {
		if (us == 0) { // Not valid
			return;
		}

		unsigned char CSMask = 0;
		// For this notes, we asume 16MHz CPU. We recalculate 'us' if not:
		if (F_CPU != 16000000) {
			us = F_CPU / 16000000 * us;
		}

		// Leonardo and other 32U4 boards
		#ifdef __AVR_ATmega32U4__
			TIMSK3 &= ~((1 << TOIE3) | (1 << OCIE3A));	// Disable overflow interruption when 0 + Disable interrupt on compare match
			cli();
			// 32U4, using Timer3. Counts at 16MHz
			/*
			Prescaler: TCCR3B; 3 last bits, CS30, CS31 and CS32

			CS32	CS31	CS30	Freq		Divisor		Base Delay	Overflow delay
			  0		  0		  0		stopped		   -		    -			    -
			  0		  0		  1		16MHz		   1		0.0625us			   16us
			  0		  1		  0		2MHz		   8		   0.5us			  128us
			  0		  1		  1		250KHz		  64		     4us			 1024us
			  1		  0		  0		62.5KHz		 256		    16us			 4096us
			  1		  0		  1		15.625KHz	1024		    64us			16384us
			*/
			if (us >= 16384) {
				CSMask = (1<<CS32) | (1<<CS31) | (1<<CS30);
				_overflows = us / 16384;
				_remaining = 256 - ((us % 16384) / 64 + 0.5); // + 0.5 is round for positive numbers
			} else {
				if (us >= 4096) {
					CSMask = (1<<CS32) | (1<<CS30);
					_remaining = 256 - (us / 64 + 0.5); // + 0.5 is round for positive numbers
				} else if (us >= 2048) {
					CSMask = (1<<CS32);
					_remaining = 256 - (us / 16 + 0.5); // + 0.5 is round for positive numbers
				} else if (us >= 512) {
					CSMask = (1<<CS31) | (1<<CS30);
					_remaining = 256 - (us / 4 + 0.5); // + 0.5 is round for positive numbers
				} else if (us >= 16) {
					CSMask = (1<<CS31);
					_remaining = 256 - us * 2;
				} else {
					CSMask = (1<<CS30);
					_remaining = 256 - (us * 16);
				}
				_overflows = 0;
			}

			__overflows = _overflows;
			__remaining = _remaining;
			_overflows += 1; // Fix interrupt incorrectly firing just after sei()
			//ASSR &= ~(1<<AS3); 		// Internal clock - Unique mode for Timer3
			TCCR3A = (1<<COM3A1);	// Normal operation
			TCCR3B = TCCR3B & ~((1<<CS32) | (1<<CS31) | (1<<CS30)) | CSMask;	// Sets divisor

			TCNT3 = 0;				// Clean timer count
			TIMSK3 |= (1 << TOIE3);		// Enable overflow interruption when 0
			sei();
		#else
			TIMSK2 &= ~((1 << TOIE2) | (1 << OCIE2A));	// Disable overflow interruption when 0 + Disable interrupt on compare match
			cli();
			// AVR, using Timer2. Counts at 16MHz
			/*
			Prescaler: TCCR2B; 3 last bits, CS20, CS21 and CS22

			CS22	CS21	CS20	Freq		Divisor		Base Delay	Overflow delay
			  0		  0		  0		stopped		   -		    -			    -
			  0		  0		  1		16MHz		   1		0.0625us			   16us
			  0		  1		  0		2MHz		   8		   0.5us			  128us
			  0		  1		  1		500KHz		  32		     2us			  512us
			  1		  0		  0		250KHz		  64		     4us			 1024us
			  1		  0		  1		125KHz		 128		     8us			 2048us
			  1		  1		  0		62.5KHz		 256		    16us			 4096us
			  1		  1		  1		15.625KHz	1024		    64us			16384us
			*/
			if (us >= 16384) {
				CSMask = (1<<CS22) | (1<<CS21) | (1<<CS20);
				_overflows = us / 16384;
				_remaining = 256 - ((us % 16384) / 64 + 0.5); // + 0.5 is round for positive numbers
			} else {
				if (us >= 4096) {
					CSMask = (1<<CS22) | (1<<CS21) | (1<<CS20);
					_remaining = 256 - (us / 64 + 0.5); // + 0.5 is round for positive numbers
				} else if (us >= 2048) {
					CSMask = (1<<CS22) | (1<<CS21);
					_remaining = 256 - (us / 16 + 0.5); // + 0.5 is round for positive numbers
				} else if (us >= 1024) {
					CSMask = (1<<CS22) | (1<<CS20);
					_remaining = 256 - (us / 8 + 0.5); // + 0.5 is round for positive numbers
				} else if (us >= 512) {
					CSMask = (1<<CS22);
					_remaining = 256 - (us / 4 + 0.5); // + 0.5 is round for positive numbers
				} else if (us >= 128) {
					CSMask = (1<<CS21) | (1<<CS20);
					_remaining = 256 - (us / 2 + 0.5); // + 0.5 is round for positive numbers
				} else if (us >= 16) {
					CSMask = (1<<CS21);
					_remaining = 256 - us * 2;
				} else {
					CSMask = (1<<CS20);
					_remaining = 256 - (us * 16);
				}
				_overflows = 0;
			}

			__overflows = _overflows;
			__remaining = _remaining;
			_overflows += 1; // Fix interrupt incorrectly firing just after sei()
			ASSR &= ~(1<<AS2); 		// Internal clock
			TCCR2A = (1<<COM2A1);	// Normal operation
			TCCR2B = TCCR2B & ~((1<<CS22) | (1<<CS21) | (1<<CS20)) | CSMask;	// Sets divisor

			TCNT2 = 0;				// Clean timer count
			TIMSK2 |= (1 << TOIE2);		// Enable overflow interruption when 0
			sei();
		#endif
	}


	/**
	 * \brief Sets up the timer, calculation variables and interrupts for desired s seconds
	 *
	 * Note: This is device-dependant
	 *
	 * @param	s		Desired timing in seconds
	 */
	void uTimerLib::_attachInterrupt_s(unsigned long int s) {
		if (s == 0) { // Not valid
			return;
		}

		unsigned char CSMask = 0;
		// For this notes, we asume 16MHz CPU. We recalculate 's' if not:
		if (F_CPU != 16000000) {
			s = F_CPU / 16000000 * s;
		}

		// Leonardo and other 32U4 boards
		#ifdef __AVR_ATmega32U4__
			TIMSK3 &= ~((1 << TOIE3) | (1 << OCIE3A));	// Disable overflow interruption when 0 + Disable interrupt on compare match
			cli();

			/*
			Using longest mode from _ms function
			CS32	CS31	CS30	Freq		Divisor		Base Delay	Overflow delay
			  1		  0		  0		15.625KHz	1024		    64us			16384us
			*/
			CSMask = (1<<CS32);
			if (_overflows > 500000) {
				_overflows = s / 16384 * 1000000;
			} else {
				_overflows = s * 1000000 / 16384;
			}
			// Original: _remaining = 256 - round(((us * 1000000) % 16384) / 64);
			// Anti-Overflow trick:
			if (s > 16384) {
				unsigned long int temp = floor(s / 16384) * 16384;
				_remaining = 256 - ((((s - temp) * 1000000) % 16384) / 64 + 0.5); // + 0.5 is round for positive numbers
			} else {
				_remaining = 256 - (((s * 1000000) % 16384) / 64 + 0.5); // + 0.5 is round for positive numbers
			}

			__overflows = _overflows;
			__remaining = _remaining;
			_overflows += 1;

			//ASSR &= ~(1<<AS3); 		// Internal clock - Unique mode for Timer3
			TCCR3A = (1<<COM3A1);	// Normal operation
			TCCR3B = TCCR3B & ~((1<<CS32) | (1<<CS31) | (1<<CS30)) | CSMask;	// Sets divisor

			// Clean counter in normal operation, load remaining when overflows == 0
			TCNT3 = 0;				// Clean timer count
			TIMSK3 |= (1 << TOIE3);		// Enable overflow interruption when 0
			sei();

		#else
			// Arduino AVR
			TIMSK2 &= ~((1 << TOIE2) | (1 << OCIE2A));	// Disable overflow interruption when 0 + Disable interrupt on compare match
			cli();

			/*
			Using longest mode from _ms function
			CS22	CS21	CS20	Freq		Divisor		Base Delay	Overflow delay
			  1		  1		  1		15.625KHz	1024		    64us			16384us
			*/
			CSMask = (1<<CS22) | (1<<CS21) | (1<<CS20);
			if (_overflows > 500000) {
				_overflows = s / 16384 * 1000000;
			} else {
				_overflows = s * 1000000 / 16384;
			}
			// Original: _remaining = 256 - round(((us * 1000000) % 16384) / 64);
			// Anti-Overflow trick:
			if (s > 16384) {
				unsigned long int temp = floor(s / 16384) * 16384;
				_remaining = 256 - ((((s - temp) * 1000000) % 16384) / 64 + 0.5); // + 0.5 is round for positive numbers
			} else {
				_remaining = 256 - (((s * 1000000) % 16384) / 64 + 0.5); // + 0.5 is round for positive numbers
			}

			__overflows = _overflows;
			__remaining = _remaining;
			_overflows += 1;

			ASSR &= ~(1<<AS2); 		// Internal clock
			TCCR2A = (1<<COM2A1);	// Normal operation
			TCCR2B = TCCR2B & ~((1<<CS22) | (1<<CS21) | (1<<CS20)) | CSMask;	// Sets divisor

			// Clean counter in normal operation, load remaining when overflows == 0
			TCNT2 = 0;				// Clean timer count
			TIMSK2 |= (1 << TOIE2);		// Enable overflow interruption when 0
			sei();
		#endif
	}



	/**
	 * \brief Loads last bit of time needed to precisely count until desired time (non complete loop)
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::_loadRemaining() {
		#ifdef __AVR_ATmega32U4__
			TCNT3 = _remaining;
		#else
			TCNT2 = _remaining;
		#endif
	}

	/**
	 * \brief Clear timer interrupts
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::clearTimer() {
		_type = UTIMERLIB_TYPE_OFF;

		#ifdef __AVR_ATmega32U4__
			TIMSK3 &= ~(1 << TOIE3);		// Disable overflow interruption when 0
			SREG = (SREG & 0b01111111); // Disable interrupts without modifiying other interrupts
		#else
			TIMSK2 &= ~(1 << TOIE2);		// Disable overflow interruption when 0
			SREG = (SREG & 0b01111111); // Disable interrupts without modifiying other interrupts
		#endif

	}

	/**
	 * \brief Internal intermediate function to control timer interrupts
	 *
	 * As timers doesn't give us enougth flexibility for large timings,
	 * this function implements oferflow control to offer user desired timings.
	 */
	void uTimerLib::_interrupt() {
		if (_type == UTIMERLIB_TYPE_OFF) { // Should not happen
			return;
		}
		if (_overflows > 0) {
			_overflows--;
		}
		if (_overflows == 0 && _remaining > 0) {
				// Load remaining count to counter
				_loadRemaining();
				// And clear remaining count
				_remaining = 0;
		} else if (_overflows == 0 && _remaining == 0) {
			if (_type == UTIMERLIB_TYPE_TIMEOUT) {
				clearTimer();
			} else if (_type == UTIMERLIB_TYPE_INTERVAL) {
				if (__overflows == 0) {
					_remaining = __remaining;
					_loadRemaining();
					_remaining = 0;
				} else {
					_overflows = __overflows;
					_remaining = __remaining;
				}
			}
			_cb();
		}
	}


	/**
	 * \brief Preinstantiate Object
	 *
	 * Now you can use al functionality calling Timerlib.function
	 */
	uTimerLib TimerLib = uTimerLib();





	/**
	 * \brief Attach Interrupts using internal functionality
	 *
	 * Note: This is device-dependant
	 */
	#ifdef __AVR_ATmega32U4__
		// Arduino AVR
		ISR(TIMER3_OVF_vect) {
			TimerLib._interrupt();
		}
	#else
		// Arduino AVR
		ISR(TIMER2_OVF_vect) {
			TimerLib._interrupt();
		}
	#endif

#endif
#endif
