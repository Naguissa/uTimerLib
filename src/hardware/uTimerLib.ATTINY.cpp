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

#if (defined(ARDUINO_ARCH_AVR) && (defined(ARDUINO_attiny) || defined(ARDUINO_AVR_ATTINYX4) || defined(ARDUINO_AVR_ATTINYX5) || defined(ARDUINO_AVR_ATTINYX7) || defined(ARDUINO_AVR_ATTINYX8) || defined(ARDUINO_AVR_ATTINYX61) || defined(ARDUINO_AVR_ATTINY43) || defined(ARDUINO_AVR_ATTINY828) || defined(ARDUINO_AVR_ATTINY1634) || defined(ARDUINO_AVR_ATTINYX313))) && defined(UTIMERLIB_HW_COMPILE)
#if	!defined(_uTimerLib_IMP_) && defined(_uTimerLib_cpp_)
	#define _uTimerLib_IMP_
	#include "uTimerLib.cpp"


	#ifndef TCCR1A
		#define TCCR1A GTCCR
	#endif


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
			us = (F_CPU / 1000) * us / 16000;
		}
		TIMSK &= ~((1 << TOIE1) | (1 << OCIE1A));	// Disable overflow interruption when 0 + Disable interrupt on compare match
		cli();
		// ATTiny, using Timer1. Counts at 16MHz (it other speed is used we recalculate us before)
		/*
		Prescaler: TCCR1; 4 last bits, CS10, CS11, CS12 and CS13

		CS13	CS12	CS11	CS10	Freq		Divisor		Base Delay		Overflow delay
		  0		  0		  0		 0		stopped		    -		    -			        -
		  0		  0		  0		 1		16MHz		    1		   0.0625us			    16us
		  0		  0		  1		 0		16MHz		    2		   0.125us			    32us
		  0		  0		  1		 1		16MHz		    4		   0.25us			    64us
		  0		  1		  0		 0		16MHz		    8		   0.5us			   128us
		  0		  1		  0		 1		16MHz		   16		   1us				   256us
		  0		  1		  1		 0		16MHz		   32		   2us				   512us
		  0		  1		  1		 1		16MHz		   64		   4us				  1024us
		  1		  0		  0		 0		16MHz		  128		   8us				  2048us
		  1		  0		  0		 1		16MHz		  256		  16us				  4096us
		  1		  0		  1		 0		16MHz		  512		  32us				  8192us
		  1		  0		  1		 1		16MHz		 1024		  64us				 16384us
		  1		  1		  0		 0		16MHz		 2048		 128us				 32768us
		  1		  1		  0		 1		16MHz		 4096		 256us				 65536us
		  1		  1		  1		 0		16MHz		 8192		 512us				131072us
		  1		  1		  1		 1		16MHz		16384		1024us				262144us
		*/
		if (us >= 262144) {
			CSMask = (1<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10);
			_overflows = us / 262144;
			_remaining = 256 - ((us % 262144) / 1024 + 0.5); // + 0.5 is round for positive numbers
		} else {
			if (us >= 131072) {
				CSMask = (1<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10);
				_remaining = 256 - (us / 1024 + 0.5); // + 0.5 is round for positive numbers
			} else if (us >= 65536) {
				CSMask = (1<<CS13) | (1<<CS12) | (1<<CS11);
				_remaining = 256 - (us / 512 + 0.5); // + 0.5 is round for positive numbers
			} else if (us >= 32768) {
				CSMask = (1<<CS13) | (1<<CS12) | (1<<CS10);
				_remaining = 256 - (us / 256 + 0.5); // + 0.5 is round for positive numbers
			} else if (us >= 16384) {
				CSMask = (1<<CS13) | (1<<CS12);
				_remaining = 256 - (us / 128 + 0.5); // + 0.5 is round for positive numbers
			} else if (us >= 8192) {
				CSMask = (1<<CS13) | (1<<CS11) | (1<<CS10);
				_remaining = 256 - (us / 64 + 0.5); // + 0.5 is round for positive numbers
			} else if (us >= 4096) {
				CSMask = (1<<CS13) | (1<<CS11);
				_remaining = 256 - (us / 32 + 0.5);
			} else if (us >= 2048) {
				CSMask = (1<<CS13) | (1<<CS10);
				_remaining = 256 - (us / 16 + 0.5);
			} else if (us >= 1024) {
				CSMask = (1<<CS13);
				_remaining = 256 - (us / 8 + 0.5);
			} else if (us >= 512) {
				CSMask = (1<<CS12) | (1<<CS11) | (1<<CS10);
				_remaining = 256 - (us / 4 + 0.5);
			} else if (us >= 256) {
				CSMask = (1<<CS12) | (1<<CS11);
				_remaining = 256 - (us / 2 + 0.5);
			} else if (us >= 128) {
				CSMask = (1<<CS12) | (1<<CS10);
				_remaining = 256 - us;
			} else if (us >= 64) {
				CSMask = (1<<CS12);
				_remaining = 256 - (us * 2);
			} else if (us >= 32) {
				CSMask = (1<<CS11) | (1<<CS10);
				_remaining = 256 - (us * 4);
			} else if (us >= 16) {
				CSMask = (1<<CS11);
				_remaining = 256 - (us * 8);
			} else {
				CSMask = (1<<CS10);
				_remaining = 256 - (us * 16);
			}
			_overflows = 0;
		}

		__overflows = _overflows;
		__remaining = _remaining;
		_overflows += 1; // Fix interrupt incorrectly firing just after sei()

		PLLCSR &= ~(1<<PCKE); 		// Internal clock
		// TCCR1A = (1<<COM1A1);	// Normal operation

		TCCR1 |= (1 << CTC1);  // clear timer on compare match
		TCCR1 = TCCR1 & ~((1<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10)) | CSMask;	// Sets divisor

		TCNT1 = 0;				// Clean timer count
		TIMSK |= (1 << TOIE1);		// Enable overflow interruption when 0
		sei();
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
		// For this notes, we asume 16MHz CPU. We recalculate 'us' if not:
		if (F_CPU != 16000000) {
			s = (F_CPU / 1000) * s / 16000;
		}
		TIMSK &= ~((1 << TOIE1) | (1 << OCIE1A));	// Disable overflow interruption when 0 + Disable interrupt on compare match
		cli();
		// ATTiny, using Timer1. Counts at 16MHz (it other speed is used we recalculate us before)
		/*
		Prescaler: TCCR1; 4 last bits, CS10, CS11, CS12 and CS13

		CS13	CS12	CS11	CS10	Freq		Divisor		Base Delay		Overflow delay
		  1		  1		  1		 1		16MHz		16384		1024us				262144us
		*/

		CSMask = (1<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10);
		_overflows = s * 1000000 / 262144;
		_remaining = 256 - (((s * 1000000) % 262144) / 1024 + 0.5); // + 0.5 is round for positive numbers

		__overflows = _overflows;
		__remaining = _remaining;
		_overflows += 1; // Fix interrupt incorrectly firing just after sei()

		PLLCSR &= ~(1<<PCKE); 		// Internal clock
		// TCCR1A = (1<<COM1A1);	// Normal operation

		// All '1', so simplify to next line... TCCR1 = TCCR1 & ~((1<<CS13) | (1<<CS12) | (1<<CS11) | (1<<CS10)) | CSMask;	// Sets divisor
		TCCR1 = TCCR1 | CSMask;	// Sets divisor

		// Clean counter in normal operation, load remaining when overflows == 0
		TCNT1 = 0;				// Clean timer count
		TIMSK |= (1 << TOIE1);		// Enable overflow interruption when 0
		sei();
	}



	/**
	 * \brief Loads last bit of time needed to precisely count until desired time (non complete loop)
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::_loadRemaining() {
		TCNT1 = _remaining;
	}

	/**
	 * \brief Clear timer interrupts
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::clearTimer() {
		_type = UTIMERLIB_TYPE_OFF;

		TIMSK &= ~(1 << TOIE1);		// Disable overflow interruption when 0
//		SREG = (SREG & 0b01111111); // Disable interrupts without modifiying other interrupts

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
	ISR(TIMER1_OVF_vect) {
		TimerLib._interrupt();
	}

#endif
#endif
