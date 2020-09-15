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
#if defined(__SAMD51__) && defined(UTIMERLIB_HW_COMPILE)
#if	!defined(_uTimerLib_IMP_) && defined(_uTimerLib_cpp_)
	#define _uTimerLib_IMP_
	#include "uTimerLib.cpp"

	#define UTIMERLIB_WAIT_SYNC() while (TC1->COUNT16.SYNCBUSY.reg)
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

		We will use TC1

		REMEMBER! 16 bit counter!!!

		Name			Prescaler	Freq		Base Delay		Overflow delay
		GCLK_TC			   1		 120MHz		0,008333333us	    546,133333333us;    0,546133333333ms
		GCLK_TC/2		   2		  60MHz		0,016666667us	   1092,266666667us;    1,092266666667ms
		GCLK_TC/4		   4		  30MHz		0,033333333us	   2184,533333333us;    2,184533333333ms
		GCLK_TC/8		   8		  20MHz		0,066666667us	   4369,066666667us;    4,369066666667ms
		GCLK_TC/16		  16		  10MHz		0,133333333us	   8738,133333333us;    8,738133333333ms
		GCLK_TC/64		  64		1,875MHz	0,533333333us	  34952,533333333us;   34,952533333333ms
		GCLK_TC/256		 256		468,75KHz	2,133333333us	 139810,133333333us;  139,810133333333ms
		GCLK_TC/1024	1024		117,1875Hz	8,533333333us	 559240,533333333us;  559,240533333333ms

		Will be using:
			GCLK_TC/16 for us
			GCLK_TC/1024 for s
		*/


/*
		// Enable the TC bus clock
		MCLK->APBAMASK.bit.TC1_ = 1;
		GCLK->PCHCTRL[TC1_GCLK_ID].bit.GEN = 0;
		GCLK->PCHCTRL[TC1_GCLK_ID].bit.CHEN = 1;
*/
		// Enable the TC bus clock
		GCLK->PCHCTRL[TC1_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK1_Val | GCLK_PCHCTRL_CHEN;
		while(GCLK->SYNCBUSY.reg); // sync

		TC1->COUNT16.CTRLA.bit.ENABLE = 0;
		UTIMERLIB_WAIT_SYNC();

		TC1->COUNT16.CTRLA.bit.MODE = TC_CTRLA_MODE_COUNT16_Val;
		UTIMERLIB_WAIT_SYNC();



		TC1->COUNT16.CTRLA.reg &= ((~(TC_CTRLA_ENABLE)) & (~(TC_CTRLA_PRESCALER_DIV1024)) & (~(TC_CTRLA_PRESCALER_DIV256)) & (~(TC_CTRLA_PRESCALER_DIV64)) & (~(TC_CTRLA_PRESCALER_DIV16)) & (~(TC_CTRLA_PRESCALER_DIV4)) & (~(TC_CTRLA_PRESCALER_DIV2)) & (~(TC_CTRLA_PRESCALER_DIV1)));
		UTIMERLIB_WAIT_SYNC();

		TC1->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV16;
		UTIMERLIB_WAIT_SYNC();


		if (us > 8738) {
			__overflows = _overflows = us / 8738.133333333;
			__remaining = _remaining = (us - (8738.133333333 * _overflows)) / 0.133333333 + 0.5; // +0.5 is same as round
		} else {
			__overflows = _overflows = 0;
			__remaining = _remaining = (us / 0.133333333 + 0.5); // +0.5 is same as round
		}

		if (__remaining != 0) {
			__remaining = _remaining = (((uint16_t) 0xffff) - __remaining); // Remaining is max value minus remaining
		}

		if (__overflows == 0) {
			_loadRemaining();
			_remaining = 0;
		} else {
			TC1->COUNT16.COUNT.reg = 0;
		}


		TC1->COUNT16.INTENSET.reg = 0;
		TC1->COUNT16.INTENSET.bit.OVF = 1;
		// Enable InterruptVector
		NVIC_EnableIRQ(TC1_IRQn);

		// Count on event
		//TC1->COUNT16.EVCTRL.bit.EVACT = TC_EVCTRL_EVACT_COUNT_Val;

		TC1->COUNT16.CTRLA.bit.ENABLE = 1;
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
		GCLK_TC/1024	1024		117,1875Hz	8,533333333us	 559240,533333333us;  559,240533333333ms
		*/




		// Enable the TC bus clock
		GCLK->PCHCTRL[TC1_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK1_Val | GCLK_PCHCTRL_CHEN;
		while(GCLK->SYNCBUSY.reg); // sync

		TC1->COUNT16.CTRLA.bit.ENABLE = 0;
		UTIMERLIB_WAIT_SYNC();

		TC1->COUNT16.CTRLA.bit.MODE = TC_CTRLA_MODE_COUNT16_Val;
		UTIMERLIB_WAIT_SYNC();



		TC1->COUNT16.CTRLA.reg &= ((~(TC_CTRLA_ENABLE)) & (~(TC_CTRLA_PRESCALER_DIV1024)) & (~(TC_CTRLA_PRESCALER_DIV256)) & (~(TC_CTRLA_PRESCALER_DIV64)) & (~(TC_CTRLA_PRESCALER_DIV16)) & (~(TC_CTRLA_PRESCALER_DIV4)) & (~(TC_CTRLA_PRESCALER_DIV2)) & (~(TC_CTRLA_PRESCALER_DIV1)));
		UTIMERLIB_WAIT_SYNC();

		TC1->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
		UTIMERLIB_WAIT_SYNC();


		__overflows = _overflows = s / 0.559240533333333;
		__remaining = _remaining = (s - (0.559240533333333 * _overflows)) / 0.000008533333333 + 0.5; // +0.5 is same as round

		if (__remaining != 0) {
			__remaining = _remaining = (((uint16_t) 0xffff) - __remaining); // Remaining is max value minus remaining
		}

		if (__overflows == 0) {
			_loadRemaining();
			_remaining = 0;
		} else {
			TC1->COUNT16.COUNT.reg = 0;
		}


		TC1->COUNT16.INTENSET.reg = 0;
		TC1->COUNT16.INTENSET.bit.OVF = 1;
		// Enable InterruptVector
		NVIC_EnableIRQ(TC1_IRQn);

		// Count on event
		//TC1->COUNT16.EVCTRL.bit.EVACT = TC_EVCTRL_EVACT_COUNT_Val;

		TC1->COUNT16.CTRLA.bit.ENABLE = 1;
	}



	/**
	 * \brief Loads last bit of time needed to precisely count until desired time (non complete loop)
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::_loadRemaining() {
		TC1->COUNT16.COUNT.reg = _remaining;
	}

	/**
	 * \brief Clear timer interrupts
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::clearTimer() {
		_type = UTIMERLIB_TYPE_OFF;

		TC1->COUNT16.INTENSET.reg = 0;
		// Disable InterruptVector
		NVIC_DisableIRQ(TC1_IRQn);
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
	void TC1_Handler() {
		if (TC1->COUNT16.INTFLAG.bit.OVF == 1) {
			TC1->COUNT16.INTENSET.bit.OVF = 1;  // Clear flag
			TimerLib._interrupt();
		}
		if (TC1->COUNT16.INTFLAG.bit.MC0 == 1) {
			TC1->COUNT16.INTENSET.bit.MC0 = 1;  // Clear flag
		}
	}

#endif
#endif
