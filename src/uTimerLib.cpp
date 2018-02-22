/**
 * Tiny and cross-device compatible timer library.
 *
 * Timers used by microcontroller
 *	Atmel AVR:	Timer2 (3rd timer)
 *  STM32:		Timer3 (3rd timer)
 *  SAM (Due):  TC3 (Timer1, channel 0)
 *
 * @copyright Naguissa
 * @author Naguissa
 * @email naguissa@foroelectro.net
 * @version 0.1.0
 * @created 2018-01-27
 */
#include "uTimerLib.h"

#ifdef _VARIANT_ARDUINO_STM32_
	uTimerLib *uTimerLib::_instance = NULL;
#endif

/**
 * Constructor
 *
 * Nothing to do here
 */
uTimerLib::uTimerLib() {
	#ifdef _VARIANT_ARDUINO_STM32_
		_instance = this;
		clearTimer();
	#endif
}

/**
 * Attaches a callback function to be executed each us microseconds
 *
 * @param	void*()				cb		Callback function to be called
 * @param	unsigned long int	us		Interval in microseconds
 */
void uTimerLib::setInterval_us(void (* cb)(), unsigned long int us) {
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
int uTimerLib::setTimeout_us(void (* cb)(), unsigned long int us) {
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
void uTimerLib::setInterval_s(void (* cb)(), unsigned long int s) {
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
int uTimerLib::setTimeout_s(void (* cb)(), unsigned long int s) {
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
	if (us == 0) { // Not valid
		return;
	}
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

		ASSR &= ~(1<<AS2); 		// Internal clock
		TCCR2A = (1<<COM2A1);	// Normal operation
		TCCR2B = TCCR2B & ~((1<<CS22) | (1<<CS21) | (1<<CS20)) | CSMask;	// Sets divisor

		// Clean counter in normal operation, load remaining when overflows == 0
		if (__overflows == 0) {
			_loadRemaining();
			_remaining = 0;
		} else {
			TCNT2 = 0;				// Clean timer count
		}
		TIMSK2 |= (1 << TOIE2);		// Enable overflow interruption when 0
    	TIMSK2 &= ~(1 << OCIE2A);	// Disable interrupt on compare match

		sei();
	#endif

	// STM32, all variants
	#ifdef _VARIANT_ARDUINO_STM32_
		Timer3.setMode(TIMER_CH1, TIMER_OUTPUTCOMPARE);
		Timer3.setPeriod(us);				// in microseconds
		Timer3.setCompare(TIMER_CH1, 1);
		__overflows = _overflows = 1;
		__remaining = _remaining = 0;
		if (_toInit) {
			_toInit = false;
			Timer3.attachInterrupt(TIMER_CH1, uTimerLib::interrupt);
		}
	    Timer3.refresh();
		Timer3.resume();
	#endif



	// SAM, Arduino Due
	#ifdef ARDUINO_ARCH_SAM
		/*
		Prescalers: MCK/2, MCK/8, MCK/32, MCK/128
		Base frequency: 84MHz

		Available Timers:
		ISR/IRQ	 TC	  Channel	Due pins
		  TC0	TC0		0		2, 13
		  TC1	TC0		1		60, 61
		  TC2	TC0		2		58
		  TC3	TC1		0		none
		  TC4	TC1		1		none
		  TC5	TC1		2		none
		  TC6	TC2		0		4, 5
		  TC7	TC2		1		3, 10
		  TC8	TC2		2		11, 12

		We will use TC1, as it has no associated pins. We will choose Channel 0, so ISR is TC3

		REMEMBER! 32 bit counter!!!


				Name				Prescaler	Freq	Base Delay		Overflow delay
		TC_CMR_TCCLKS_TIMER_CLOCK1	  2		    42MHz   0,023809524us	102261126,913327104us, 102,261126913327104s
		TC_CMR_TCCLKS_TIMER_CLOCK2	  8		  10.5MHz	0,095238095us	409044503,35834112us, 409,04450335834112s
		TC_CMR_TCCLKS_TIMER_CLOCK3	 32		 2.625MHz	0,380952381us	1636178017,523809524us, 1636,178017523809524s
		TC_CMR_TCCLKS_TIMER_CLOCK4	128		656.25KHz	1,523809524us	6544712070,913327104us, 6544,712070913327104s

		For simplify things, we'll use always TC_CMR_TCCLKS_TIMER_CLOCK32,as has enougth resolution for us.


		*/
		if (us > 1636178017) {
			__overflows = _overflows = us / 1636178017.523809524;
			__remaining = _remaining = (us - (1636178017.523809524 * _overflows)) / 0.380952381 + 0.5; // +0.5 is same as round
		} else {
			__overflows = _overflows = 0;
			__remaining = _remaining = (us / 0.380952381 + 0.5); // +0.5 is same as round
		}
		pmc_set_writeprotect(false); // Enable write
		pmc_enable_periph_clk((uint32_t) TC3_IRQn); // Enable TC1 - channel 0 peripheral
		TC_Configure(TC1, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK3); // Configure clock; prescaler = 32

		if (__overflows == 0) {
			_loadRemaining();
			_remaining = 0;
		} else {
			TC_SetRC(TC1, 0, 4294967295); // Int on last number
		}

		TC_Start(TC1, 0);
		TC1->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
		TC1->TC_CHANNEL[0].TC_IDR = ~TC_IER_CPCS;
		NVIC_EnableIRQ(TC3_IRQn);
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
	if (s == 0) { // Not valid
		return;
	}
	// Arduino AVR
	#ifdef ARDUINO_ARCH_AVR
		unsigned char CSMask = 0;
		// For this notes, we asume 16MHz CPU. We recalculate 's' if not:
		if (F_CPU != 16000000) {
			s = F_CPU / 16000000 * s;
		}

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


	// STM32, all variants
	#ifdef _VARIANT_ARDUINO_STM32_
		Timer3.setMode(TIMER_CH1, TIMER_OUTPUTCOMPARE);
		Timer3.setPeriod((unsigned long int) 1000000);			// 1s, in microseconds
		Timer3.setCompare(TIMER_CH1, 1);
		__overflows = _overflows = s;
		__remaining = _remaining = 0;
		if (_toInit) {
			_toInit = false;
			Timer3.attachInterrupt(TIMER_CH1, uTimerLib::interrupt);
		}
	    Timer3.refresh();
		Timer3.resume();
	#endif


	// SAM, Arduino Due
	#ifdef ARDUINO_ARCH_SAM
		/*

		See _ms functions for detailed info; here only selected points

				Name				Prescaler	Freq	Base Delay		Overflow delay
		TC_CMR_TCCLKS_TIMER_CLOCK4	128		656.25KHz	1,523809524us	6544712070,913327104us, 6544,712070913327104s

		For simplify things, we'll use always TC_CMR_TCCLKS_TIMER_CLOCK32,as has enougth resolution for us.
		*/
		if (s > 6544) {
			__overflows = _overflows = s / 6544.712070913327104;
			__remaining = _remaining = (s - (6544.712070913327104 * _overflows) / 0.000001523809524 + 0.5); // +0.5 is same as round
		} else {
			__overflows = _overflows = 0;
			__remaining = _remaining = (s / 0.000001523809524 + 0.5); // +0.5 is same as round
		}

		pmc_set_writeprotect(false); // Enable write
		//pmc_enable_periph_clk((uint32_t) TC3_IRQn); // Enable TC1 - channel 0 peripheral
		pmc_enable_periph_clk(ID_TC3); // Enable TC1 - channel 0 peripheral
		TC_Configure(TC1, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4); // Configure clock; prescaler = 128

		if (__overflows == 0) {
			_loadRemaining();
			_remaining = 0;
		} else {
			TC_SetRC(TC1, 0, 4294967295); // Int on last number
		}

		TC1->TC_CHANNEL[0].TC_IER=TC_IER_CPCS;
		TC1->TC_CHANNEL[0].TC_IDR=~TC_IER_CPCS;
		NVIC_EnableIRQ(TC3_IRQn);
		TC_Start(TC1, 0);
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

	// STM32: Not needed

	#ifdef ARDUINO_ARCH_SAM
		TC_SetRC(TC1, 0, _remaining);
	#endif
}

/**
 * Clear timer interrupts
 *
 * Note: This is device-dependant
 */
void uTimerLib::clearTimer() {
	_type = UTIMERLIB_TYPE_OFF;

	#ifdef ARDUINO_ARCH_AVR
		TIMSK2 &= ~(1 << TOIE2);		// Disable overflow interruption when 0
		SREG = (SREG & 0b01111111); // Disable interrupts without modifiying other interrupts
	#endif

	#ifdef _VARIANT_ARDUINO_STM32_
		Timer3.pause();
	#endif

	// SAM, Arduino Due
	#ifdef ARDUINO_ARCH_SAM
        NVIC_DisableIRQ(TC3_IRQn);
	#endif

}

/**
 * Internal intermediate function to control timer interrupts
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
				#ifdef ARDUINO_ARCH_SAM
					TC_SetRC(TC1, 0, 4294967295);
				#endif
			}
		}
		_cb();
	}
	#ifdef ARDUINO_ARCH_SAM
		// Reload for SAM
		else if (_overflows > 0) {
			TC_SetRC(TC1, 0, 4294967295);
		}
	#endif

}

/**
 * Static envelope for Internal intermediate function to control timer interrupts
 */
#ifdef _VARIANT_ARDUINO_STM32_
	void uTimerLib::interrupt() {
		_instance->_interrupt();
	}
#endif


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
		TimerLib._interrupt();
	}
#endif


#ifdef ARDUINO_ARCH_SAM
	void TC3_Handler() {
		TC_GetStatus(TC1, 0); // reset interrupt
		TimerLib._interrupt();
	}
#endif

