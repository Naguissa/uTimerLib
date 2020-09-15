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
#if (defined(_VARIANT_ARDUINO_STM32_) || defined(ARDUINO_ARCH_STM32)) && defined(UTIMERLIB_HW_COMPILE)
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

		// STM32, all variants - Max us is: uint32 max / CYCLES_PER_MICROSECOND
		// ST's Arduino Core STM32, https://github.com/stm32duino/Arduino_Core_STM32
		#ifdef BOARD_NAME
			Timer3->setMode(1, TIMER_OUTPUT_COMPARE);
			__overflows = _overflows = __remaining = _remaining = 0;
			Timer3->setOverflow(us, MICROSEC_FORMAT);
			Timer3->setCaptureCompare(1, us - 1, MICROSEC_COMPARE_FORMAT);
			if (_toInit) {
				_toInit = false;
				Timer3->attachInterrupt(1, uTimerLib::interrupt);
			}
			Timer3->resume();

		// Roger Clark Arduino STM32, https://github.com/rogerclarkmelbourne/Arduino_STM32
		#else
			Timer3.setMode(TIMER_CH1, TIMER_OUTPUTCOMPARE);
			__overflows = _overflows = __remaining = _remaining = 0;
			uint16_t timerOverflow = Timer3.setPeriod(us);
			Timer3.setCompare(TIMER_CH1, timerOverflow);
			if (_toInit) {
				_toInit = false;
				Timer3.attachInterrupt(TIMER_CH1, uTimerLib::interrupt);
			}
			Timer3.refresh();
			Timer3.resume();
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

		// ST's Arduino Core STM32, https://github.com/stm32duino/Arduino_Core_STM32
		#ifdef BOARD_NAME
			Timer3->setMode(1, TIMER_OUTPUT_COMPARE);
			__overflows = _overflows = s;
			__remaining = _remaining = 0;
			Timer3->setOverflow((unsigned long int) 1000000, MICROSEC_FORMAT);
			Timer3->setCaptureCompare(1, (unsigned long int) 1000000, MICROSEC_COMPARE_FORMAT);
			if (_toInit) {
				_toInit = false;
				Timer3->attachInterrupt((uint32_t) 1, uTimerLib::interrupt);
			}
			Timer3->resume();

		// Roger Clark Arduino STM32, https://github.com/rogerclarkmelbourne/Arduino_STM32
		#else
			Timer3.setMode(TIMER_CH1, TIMER_OUTPUTCOMPARE);
			Timer3.setPeriod((uint32) 1000000);
			Timer3.setCompare(TIMER_CH1, 0);
			Timer3.setCount(1);
			__overflows = _overflows = s;
			__remaining = _remaining = 0;
			if (_toInit) {
				_toInit = false;
				Timer3.attachInterrupt(TIMER_CH1, uTimerLib::interrupt);
			}
			Timer3.refresh();
			Timer3.resume();
		#endif
	}



	/**
	 * \brief Loads last bit of time needed to precisely count until desired time (non complete loop)
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::_loadRemaining() { }

	/**
	 * \brief Clear timer interrupts
	 *
	 * Note: This is device-dependant
	 */
	void uTimerLib::clearTimer() {
		_type = UTIMERLIB_TYPE_OFF;

		// ST's Arduino Core STM32, https://github.com/stm32duino/Arduino_Core_STM32
		#ifdef BOARD_NAME
			Timer3->pause();

		// Roger Clark Arduino STM32, https://github.com/rogerclarkmelbourne/Arduino_STM32
		#else
			Timer3.pause();
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
		// We are making X overflows, as much as seconds (none on us). So wee compare upper than 1
		if (_overflows > 1) {
			_overflows--;
		} else {
			_overflows = __overflows;
			if (_type == UTIMERLIB_TYPE_TIMEOUT) {
				clearTimer();
			}
			_cb();
		}
	}

	/**
	 * \brief Static envelope for Internal intermediate function to control timer interrupts
	 */
	// ST's Arduino Core STM32, https://github.com/stm32duino/Arduino_Core_STM32
	#ifdef BOARD_NAME
		callback_function_t uTimerLib::interrupt() {
			_instance->_interrupt();
			return (callback_function_t) 0;
		}

	// Roger Clark Arduino STM32, https://github.com/rogerclarkmelbourne/Arduino_STM32
	#else
		void uTimerLib::interrupt() {
			_instance->_interrupt();
		}
	#endif



	/**
	 * \brief Preinstantiate Object
	 *
	 * Now you can use al functionality calling Timerlib.function
	 */
	uTimerLib TimerLib = uTimerLib();

#endif
#endif
