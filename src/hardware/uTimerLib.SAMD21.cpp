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
#if defined(_SAMD21_) && defined(UTIMERLIB_HW_COMPILE)
#ifndef _uTimerLib_IMP_
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

		/*
		16 bit timer

		Prescaler:
		Prescalers: GCLK_TC, GCLK_TC/2, GCLK_TC/4, GCLK_TC/8, GCLK_TC/16, GCLK_TC/64, GCLK_TC/256, GCLK_TC/1024
		Base frequency: 84MHz

		We will use TCC2, as there're some models with only 3 timers (regular models have 5 TCs)

		REMEMBER! 16 bit counter!!!


		Name			Prescaler	Freq		Base Delay		Overflow delay
		GCLK_TC			   1		 48MHz		0,020833333us	   1365,333333333us;    1,365333333333ms
		GCLK_TC/2		   2		 24MHz		0,041666667us	   2730,666666667us;    2,730666666667ms
		GCLK_TC/4		   4		 12MHz		0,083333333us	   5461,333333333us;    5,461333333333ms
		GCLK_TC/8		   8		  6MHz		0,166666667us	  10922,666666667us;   10,922666666667ms
		GCLK_TC/16		  16		  3MHz		0,333333333us	  21845,333333333us;   21,845333333333ms
		GCLK_TC/64		  64		750KHz		1,333333333us	  87381,333311488us;   87,381333311488ms
		GCLK_TC/256		 256		187,5KHz	5,333333333us	 349525,333311488us;  349,525333311488ms
		GCLK_TC/1024	1024		46.875Hz	21,333333333us	1398101,333333333us; 1398,101333333333ms; 1,398101333333333s

		Will be using:
			GCLK_TC/16 for us
			GCLK_TC/1024 for s
		*/

		// Enable clock for TC
		REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3)) ;
		while (GCLK->STATUS.bit.SYNCBUSY == 1); // sync

		// Disable TC
		_TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
		while (_TC->STATUS.bit.SYNCBUSY == 1); // sync

		// Set Timer counter Mode to 16 bits + Set TC as normal Normal Frq + Prescaler: GCLK_TC/16
		_TC->CTRLA.reg |= (TC_CTRLA_MODE_COUNT16 + TC_CTRLA_WAVEGEN_NFRQ + TC_CTRLA_PRESCALER_DIV16);
		while (_TC->STATUS.bit.SYNCBUSY == 1); // sync

		if (us > 21845) {
			__overflows = _overflows = us / 21845.333333333;
			__remaining = _remaining = ((us - (21845.333333333 * _overflows)) / 0.333333333) + 0.5; // +0.5 is same as round
		} else {
			__overflows = _overflows = 0;
			__remaining = _remaining = (us / 0.333333333) + 0.5; // +0.5 is same as round
		}

		if (__overflows == 0) {
			_loadRemaining();
			_remaining = 0;
		} else {
			_TC->CC[0].reg = 65535;
			_TC->INTENSET.reg = 0;              // disable all interrupts
			_TC->INTENSET.bit.OVF = 1;          // enable overfollow
			// Skip: while (_TC->STATUS.bit.SYNCBUSY == 1); // sync
		}

		_TC->COUNT.reg = 0;              // Reset to 0

		NVIC_EnableIRQ(TC3_IRQn);

		// Enable TC
		_TC->CTRLA.reg |= TC_CTRLA_ENABLE;
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

		/*
		GCLK_TC/1024	1024		46.875Hz	21,333333333us	1398101,333333333us; 1398,101333333333ms; 1,398101333333333s
		*/

		// Enable clock for TC
		REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3)) ;
		while (GCLK->STATUS.bit.SYNCBUSY == 1); // sync

		// Disable TC
		_TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
		while (_TC->STATUS.bit.SYNCBUSY == 1); // sync

		// Set Timer counter Mode to 16 bits + Set TC as normal Normal Frq + Prescaler: GCLK_TC/1024
		_TC->CTRLA.reg |= (TC_CTRLA_MODE_COUNT16 + TC_CTRLA_WAVEGEN_NFRQ + TC_CTRLA_PRESCALER_DIV1024);
		while (_TC->STATUS.bit.SYNCBUSY == 1); // sync

		if (s > 1) {
			__overflows = _overflows = s / 1.398101333333333;
			__remaining = _remaining = ((s - (1.398101333333333 * _overflows)) / 0.000021333333333) + 0.5; // +0.5 is same as round
		} else {
			__overflows = _overflows = 0;
			__remaining = _remaining = (s / 0.000021333333333) + 0.5; // +0.5 is same as round
		}

		if (__overflows == 0) {
			_loadRemaining();
			_remaining = 0;
		} else {
			_TC->CC[0].reg = 65535;
			_TC->INTENSET.reg = 0;              // disable all interrupts
			_TC->INTENSET.bit.OVF = 1;          // enable overfollow
			// Skip: while (_TC->STATUS.bit.SYNCBUSY == 1); // sync
		}

		_TC->COUNT.reg = 0;              // Reset to 0

		NVIC_EnableIRQ(TC3_IRQn);

		// Enable TC
		_TC->CTRLA.reg |= TC_CTRLA_ENABLE;
	}


	/**
	 * \brief Loads last bit of time needed to precisely count until desired time (non complete loop)
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::_loadRemaining() {
		_TC->COUNT.reg = 0;              // Reset to 0
		_TC->CC[0].reg = _remaining;
		_TC->INTENSET.reg = 0;              // disable all interrupts
		_TC->INTENSET.bit.MC0 = 1;          // enable compare match to CC0
		while (_TC->STATUS.bit.SYNCBUSY == 1); // sync
	}

	/**
	 * \brief Clear timer interrupts
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::clearTimer() {
		_type = UTIMERLIB_TYPE_OFF;

		// Disable TC
		_TC->INTENSET.reg = 0;              // disable all interrupts
		_TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
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

					_TC->COUNT.reg = 0;              // Reset to 0
					_TC->INTENSET.reg = 0;              // disable all interrupts
					_TC->INTENSET.bit.OVF = 0;          // enable overfollow
					_TC->INTENSET.bit.MC0 = 1;          // disable compare match to CC0
					_TC->CC[0].reg = 65535;
				}
			}
			_cb();
		} else if (_overflows > 0) { // Reload for SAMD21
			_TC->COUNT.reg = 0;              // Reset to 0
			_TC->INTENSET.reg = 0;              // disable all interrupts
			_TC->INTENSET.bit.OVF = 0;          // enable overfollow
			_TC->INTENSET.bit.MC0 = 1;          // disable compare match to CC0
			_TC->CC[0].reg = 65535;
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
	void TC3_Handler() {
		// Overflow - Nothing, we will use compare to max value instead to unify behaviour
		if (TimerLib._TC->INTFLAG.bit.OVF == 1) {
			TimerLib._TC->INTFLAG.bit.OVF = 1;  // Clear flag
			TimerLib._interrupt();
		}
		// Compare
		if (TimerLib._TC->INTFLAG.bit.MC0 == 1) {
			TimerLib._TC->INTFLAG.bit.MC0 = 1;  // Clear flag
			TimerLib._interrupt();
		}
	}

#endif
#endif
