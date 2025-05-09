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
		Base frequency: 48MHz

		We will use TCC2, as there're some models with only 3 timers (regular models have 5 TCs)

		REMEMBER! 16 bit counter!!!


		Name			Prescaler	Freq		Base Delay		Overflow delay
		GCLK_TC			   1		 48MHz		~0,020833333us	   1365,3125us;    1,3653125ms
		GCLK_TC/2		   2		 24MHz		~0,041666667us	   2730,625us;    2,730625ms
		GCLK_TC/4		   4		 12MHz		~0,083333333us	   5461,25us;    5,46125ms
		GCLK_TC/8		   8		  6MHz		~0,166666667us	  10922,5us;   10,9225ms
		GCLK_TC/16		  16		  3MHz		~0,333333333us	  21845us;   21,845ms
		GCLK_TC/64		  64		750KHz		~1,333333333us	  87380us;   87,380ms
		GCLK_TC/256		 256		187,5KHz	~5,333333333us	 349520us;  349,52ms
		GCLK_TC/1024	1024		46.875kHz	~21,333333333us	1398080us; 1398,08ms; 1,39808s

		In general:
			freq = 48 MHz / prescaler
			base_delay = 1 / freq
			overflow_delay = UINT16_MAX * base_delay

		Will be using:
			GCLK_TC/16 for us
			GCLK_TC/1024 for s

		GCLK_TC/16:
		freq = 48 MHz / prescaler = 48 MHz / 16 = 3 MHz
		base_delay = 1 / freq = 1 / 3e6 s = 1/3 us ~= 0.333333333 us
		overflow_delay = UINT16_MAX * base_delay = 65535 / 3 us = 21845 us

		GCLK_TC/1024:
		freq = 48 MHz / prescaler = 48 MHz / 1024 = 46.875 kHz = 46875 Hz
		base_delay = 1 / freq = 1 / 46875 s = ~= 21.333333333us
		overflow_delay = UINT16_MAX * base_delay = 65535 / 46875 s = 1.39808 s
		*/

		// Enable clock for TC
		REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3)) ;
		while (GCLK->STATUS.bit.SYNCBUSY == 1); // sync

		// Disable TC
		_TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
		while (_TC->STATUS.bit.SYNCBUSY == 1); // sync

		// Set Timer counter Mode to 16 bits + Set TC as normal Match Frq + Prescaler: GCLK_TC/16
		_TC->CTRLA.reg |= (TC_CTRLA_MODE_COUNT16 + TC_CTRLA_WAVEGEN_MFRQ + TC_CTRLA_PRESCALER_DIV16);
		while (_TC->STATUS.bit.SYNCBUSY == 1); // sync

		if (us > 21845) {
			__overflows = _overflows = us / 21845.0;
			__remaining = _remaining = (us - (21845 * _overflows)) * 3 - 1;
		} else {
			__overflows = _overflows = 0;
			__remaining = _remaining = us * 3 - 1;
		}

		if (__overflows == 0) {
			_loadRemaining();
			_remaining = 0;
		} else {
			_TC->CC[0].reg = UINT16_MAX;
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
		GCLK_TC/1024	1024		46.875kHz	~21,333333333us	1398080us; 1398,08ms; 1,39808s
		*/

		// Enable clock for TC
		REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3)) ;
		while (GCLK->STATUS.bit.SYNCBUSY == 1); // sync

		// Disable TC
		_TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
		while (_TC->STATUS.bit.SYNCBUSY == 1); // sync

		// Set Timer counter Mode to 16 bits + Set TC as normal Match Frq + Prescaler: GCLK_TC/1024
		_TC->CTRLA.reg |= (TC_CTRLA_MODE_COUNT16 + TC_CTRLA_WAVEGEN_MFRQ + TC_CTRLA_PRESCALER_DIV1024);
		while (_TC->STATUS.bit.SYNCBUSY == 1); // sync

		if (s > 1) {
			__overflows = _overflows = s / 1.39808;
			__remaining = _remaining = ((s * 100000) % 139808) * 480 / 1024 - 1; // for integer s this is always an integer
		} else {
			__overflows = _overflows = 0;
			__remaining = _remaining = s * 46875 - 1;
		}

		if (__overflows == 0) {
			_loadRemaining();
			_remaining = 0;
		} else {
			_TC->CC[0].reg = UINT16_MAX;
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
				if (__overflows != 0) {
					_overflows = __overflows;
					_remaining = __remaining;

					_TC->INTENSET.reg = 0;              // disable all interrupts
					_TC->INTENSET.bit.OVF = 0;          // enable overfollow
					_TC->INTENSET.bit.MC0 = 1;          // disable compare match to CC0
					_TC->CC[0].reg = UINT16_MAX;
				}
			}
			_cb();
		} else if (_overflows > 0) { // Reload for SAMD21
			_TC->INTENSET.reg = 0;              // disable all interrupts
			_TC->INTENSET.bit.OVF = 0;          // enable overfollow
			_TC->INTENSET.bit.MC0 = 1;          // disable compare match to CC0
			_TC->CC[0].reg = UINT16_MAX;
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
