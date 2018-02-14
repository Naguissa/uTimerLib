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
#include <Arduino.h>
#include "uTimerLib.h"


/**
 * Constructor
 *
 * Nothing to do here
 */
uTimerLib::uTimerLib() {}

/**
 * Attaches a callback function to be executed each us microseconds
 *
 * @param	void*()				cb		Callback function to be called
 * @param	unsigned long int	us		Interval in microseconds
 */
void uTimerLib::setInterval_us(void * cb(), unsigned long int us) {
	clearTimer();
	_cb = cb;
	_type = UTIMERLIB_TYPE_INTERVAL;
	_attachInterrupt_us(us);
}


/**
 * Attaches a callback function to be executed once when us microseconds have passed
 *
 * @param	void*()				cb		Callback function to be called
 * @param	unsigned long int	us		Timeout in microseconds
 */
int uTimerLib::setTimeout_us(void * cb(), unsigned long int us) {
	clearTimer();
	_cb = cb;
	_type = UTIMERLIB_TYPE_TIMEOUT;
	_attachInterrupt_us(us);
}


/**
 * Attaches a callback function to be executed each s seconds
 *
 * @param	void*()				cb		Callback function to be called
 * @param	unsigned long int	s		Interval in seconds
 */
void uTimerLib::setInterval_s(void * cb(), unsigned long int s) {
	clearTimer();
	_cb = cb;
	_type = UTIMERLIB_TYPE_INTERVAL;
	_attachInterrupt_s(s);
}


/**
 * Attaches a callback function to be executed once when s seconds have passed
 *
 * @param	void*()				cb		Callback function to be called
 * @param	unsigned long int	s		Timeout in seconds
 */
int uTimerLib::setTimeout_s(void * cb(), unsigned long int s) {
	clearTimer();
	_cb = cb;
	_type = UTIMERLIB_TYPE_TIMEOUT;
	_attachInterrupt_s(s);
}









/**
 * Sets up the timer, calculation variables and interrupts for desired ms microseconds
 *
 * Note: This is device-dependant
 *
 * @param	unsigned long int	us		Desired timing in microseconds
 */
void uTimerLib::_attachInterrupt_us(unsigned long int us) {
	#ifdef ARDUINO_ARCH_AVR
		unsigned char CSMask = 0;
		// For this notes, we asume 16MHz CPU. We recalculate 'us' if not:
		if (F_CPU != 16000000) {
			us = F_CPU / 16000000 * us;
		}

		cli();

		// AVR, using Timer2. Counts at 8MHz
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
			_remaining = 256 - round((us % 16384) / 64);
		} else {
			if (us >= 4096) {
				CSMask = (1<<CS22) | (1<<CS21) | (1<<CS20);
				_remaining = 256 - round(us / 64);
			} else if (us >= 2048) {
				CSMask = (1<<CS22) | (1<<CS21);
				_remaining = 256 - round(us / 16);
			} else if (us >= 1024) {
				CSMask = (1<<CS22) | (1<<CS20);
				_remaining = 256 - round(us / 8);
			} else if (us >= 512) {
				CSMask = (1<<CS22);
				_remaining = 256 - round(us / 4);
			} else if (us >= 128) {
				CSMask = (1<<CS21) | (1<<CS20);
				_remaining = 256 - round(us / 2);
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

		ASSR &= ~(1<<AS2); 		// Internal clock
		TCCR2A = (1<<COM2A1);	// Normal operation
		TCCR2B = TCCR2B & ~((1<<CS22) | (1<<CS21) | (1<<CS20)) | CSMask;	// Sets divisor

		// Clean counter in normal operation, load remaining when overflows == 0
		if (__overflows == 0) {
			_loadRemaining();
		} else {
			TCNT2 = 0;				// Clean timer count
		}
		TIMSK2 |= (1 << TOIE2);		// Enable overflow interruption when 0
    	TIMSK2 &= ~(1 << OCIE2A);	// Disable interrupt on compare match

		sei();
	#endif
}


/**
 * Sets up the timer, calculation variables and interrupts for desired s seconds
 *
 * Note: This is device-dependant
 *
 * @param	unsigned long int	s		Desired timing in seconds
 */
void uTimerLib::_attachInterrupt_s(unsigned long int s) {
	// Using longest mode from _ms function
	#ifdef ARDUINO_ARCH_AVR
		unsigned char CSMask = 0;
		// For this notes, we asume 16MHz CPU. We recalculate 's' if not:
		if (F_CPU != 16000000) {
			s = F_CPU / 16000000 * s;
		}

		cli();

		/*
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
			_remaining = 256 - round((((s - temp) * 1000000) % 16384) / 64);
		} else {
			_remaining = 256 - round(((s * 1000000) % 16384) / 64);
		}


		__overflows = _overflows;
		__remaining = _remaining;

		ASSR &= ~(1<<AS2); 		// Internal clock
		TCCR2A = (1<<COM2A1);	// Normal operation
		TCCR2B = TCCR2B & ~((1<<CS22) | (1<<CS21) | (1<<CS20)) | CSMask;	// Sets divisor

		// Clean counter in normal operation, load remaining when overflows == 0
		if (__overflows == 0) {
			_loadRemaining();
		} else {
			TCNT2 = 0;				// Clean timer count
		}
		TIMSK2 |= (1 << TOIE2);		// Enable overflow interruption when 0
    	TIMSK2 &= ~(1 << OCIE2A);	// Disable interrupt on compare match

		sei();
	#endif
}



/**
 * Loads last bit of time needed to precisely count until desired time (non complete loop)
 *
 * Note: This is device-dependant
 */
void uTimerLib::_loadRemaining() {
	#ifdef ARDUINO_ARCH_AVR
		TCNT2 = _remaining;
	#endif
}

/**
 * Clear timer interrupts
 *
 * Note: This is device-dependant
 */
void uTimerLib::clearTimer() {
	#ifdef ARDUINO_ARCH_AVR
		TIMSK2 &= ~(1 << TOIE2);		// Disable overflow interruption when 0
		SREG = (SREG & 0b01111111); // Disable interrupts without modifiying other interrupts
	#endif
	_type = UTIMERLIB_TYPE_OFF;
}



/**
 * Internal intermediate function to control timer interrupts
 *
 * As timers doesn't give us enougth flexibility for large timings,
 * this function implements oferflow control to offer user desired timings.
 */
void uTimerLib::_interruptHandler() {
	extern uTimerLib TimerLib;

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
				_loadRemaining();
			} else {
				_overflows = __overflows;
				_remaining = __remaining;
			}
		}
		_cb();
	}
}

// Preinstantiate Object
uTimerLib TimerLib = uTimerLib();




/**
 * Attach Interrupts using internal functionality
 *
 * Note: This is device-dependant
 */
#ifdef ARDUINO_ARCH_AVR
	// Arduino AVR
	ISR(TIMER2_OVF_vect) {
		TimerLib._interruptHandler();
	}
#endif

