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

#if defined(ARDUINO_ARCH_AVR) && defined(ARDUINO_AVR_DIGISPARK) && defined(UTIMERLIB_HW_COMPILE)
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
		TIMSK &= ~((1 << TOIE0) | (1 << OCIE1A));	// Disable overflow interruption when 0 + Disable interrupt on compare match
		cli();
		// Disgispark AVR, using Timer0. Counts at 16MHz (it other speed is used we recalculate us before)
		/*
		Prescaler: TCCR0B; 3 last bits, CS00, CS01 and CS02

		CS02	CS01	CS00	Freq		Divisor		Base Delay		Overflow delay
		0		  0		 0		stopped		    -		    -			        -
		0		  0		 1		16MHz		    1		   0.0625us			    16us
		0		  1		 0		16MHz		    8		   0.5us			   128us
		0		  1		 1		16MHz		   64		   4us				  1024us
		1		  0		 0		16MHz		  256		  16us				  4096us
		1		  0		 1		16MHz		 1024		  64us				 16384us
		*/
		if (us >= 16384) {
			CSMask = (1<<CS02) | (1<<CS00);
			_overflows = us / 16384;
			_remaining = 256 - ((us % 16384) / 64 + 0.5); // + 0.5 is round for positive numbers
		} else {
			if (us >= 4096) {
				CSMask = (1<<CS02) | (1<<CS00);
				_remaining = 256 - (us / 64 + 0.5); // + 0.5 is round for positive numbers
			} else if (us >= 1024) {
				CSMask = (1<<CS02);
				_remaining = 256 - (us / 16 + 0.5); // + 0.5 is round for positive numbers
			} else if (us >= 128) {
				CSMask = (1<<CS01) | (1<<CS00);
				_remaining = 256 - (us / 4 + 0.5); // + 0.5 is round for positive numbers
			} else if (us >= 16) {
				CSMask = (1<<CS01);
				_remaining = 256 - (us * 2 + 0.5); // + 0.5 is round for positive numbers
			} else {
				CSMask = (1<<CS00);
				_remaining = 256 - (us * 16);
			}
			_overflows = 0;
		}

		__overflows = _overflows;
		__remaining = _remaining;
		_overflows += 1; // Fix interrupt incorrectly firing just after sei()

		PLLCSR &= ~(1<<PCKE); 		// Internal clock

		// TCCR0A = (1 << WGM01);             //CTC mode
		// TCCR0B |= (1 << CTC);  // clear timer on compare match
		TCCR0B = TCCR0B & ~((1<<CS02) | (1<<CS01) | (1<<CS00)) | CSMask;	// Sets divisor

		TCNT0 = 0;				// Clean timer count
		TIMSK |= (1 << TOIE0);		// Enable overflow interruption when 0
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
			s = F_CPU / 16000000 * s;
		}
		TIMSK &= ~((1 << TOIE0) | (1 << OCIE1A));	// Disable overflow interruption when 0 + Disable interrupt on compare match
		cli();
		// Disgispark AVR, using Timer0. Counts at 16MHz (it other speed is used we recalculate us before)
		/*
		Prescaler: TCCR0B; 4 last bits, CS00, CS01, CS02 and CS03

		CS02	CS01	CS00	Freq		Divisor		Base Delay		Overflow delay
		1		  0		 1		16MHz		 1024		  64us				 16384us
		*/

		CSMask = (1<<CS02) | (1<<CS00);
		_overflows = s * 1000000 / 16384;
		_remaining = 256 - (((s * 1000000) % 16384) / 64 + 0.5); // + 0.5 is round for positive numbers

		__overflows = _overflows;
		__remaining = _remaining;

		PLLCSR &= ~(1<<PCKE); 		// Internal clock
		//		TCCR0A = (1 << WGM01);             //CTC mode
		// Al 1 except one, so simplify to next line...  TCCR0B = TCCR0B & ~((1<<CS02) | (1<<CS01) | (1<<CS00)) | CSMask;	// Sets divisor
		TCCR0B = TCCR0B & ~(1<<CS01) | CSMask;	// Sets divisor

		// Clean counter in normal operation, load remaining when overflows == 0
		TCNT0 = 0;				// Clean timer count
		TIMSK |= (1 << TOIE0);		// Enable overflow interruption when 0
		sei();
	}



	/**
	 * \brief Loads last bit of time needed to precisely count until desired time (non complete loop)
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::_loadRemaining() {
		TCNT0 = _remaining;
	}

	/**
	 * \brief Clear timer interrupts
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::clearTimer() {
		_type = UTIMERLIB_TYPE_OFF;

		TIMSK &= ~(1 << TOIE0);		// Disable overflow interruption when 0
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
	ISR(TIMER0_OVF_vect) {
		TimerLib._interrupt();
	}

#endif
#endif
