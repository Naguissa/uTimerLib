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
#if defined(ARDUINO_ARCH_SAM)  && defined(UTIMERLIB_HW_COMPILE)
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
		pmc_enable_periph_clk(ID_TC3); // Enable TC1 - channel 0 peripheral
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
	}


	/**
	 * \brief Loads last bit of time needed to precisely count until desired time (non complete loop)
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::_loadRemaining() {
		TC_SetRC(TC1, 0, _remaining);
	}

	/**
	 * \brief Clear timer interrupts
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::clearTimer() {
		_type = UTIMERLIB_TYPE_OFF;

        NVIC_DisableIRQ(TC3_IRQn);
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

					TC_SetRC(TC1, 0, 4294967295);
				}
			}
			_cb();
		} else if (_overflows > 0) { // Reload for SAM
			TC_SetRC(TC1, 0, 4294967295);
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
		TC_GetStatus(TC1, 0); // reset interrupt
		TimerLib._interrupt();
	}

#endif
#endif
