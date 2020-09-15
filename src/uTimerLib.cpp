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

// # if !defined(_uTimerLib_cpp_) && defined(_uTimerLib_IMP_)
#ifndef _uTimerLib_cpp_
    #define _uTimerLib_cpp_
    #include "uTimerLib.h"

    // extern uTimerLib TimerLib;

    #if defined(_VARIANT_ARDUINO_STM32_) || defined(ARDUINO_ARCH_STM32)
            uTimerLib *uTimerLib::_instance = NULL;
    #endif

    /**
     * \brief Constructor
     */
    uTimerLib::uTimerLib() {
            #ifdef _VARIANT_ARDUINO_STM32_
                    _instance = this;
                    clearTimer();
            #endif
    }

    /**
     * \brief Attaches a callback function to be executed each us microseconds
     *
     * @param	cb		Callback function to be called
     * @param	us		Interval in microseconds
     */
    void uTimerLib::setInterval_us(void (* cb)(), unsigned long int us) {
            clearTimer();
            _cb = cb;
            _type = UTIMERLIB_TYPE_INTERVAL;
            _attachInterrupt_us(us);
    }


    /**
     * \brief Attaches a callback function to be executed once when us microseconds have passed
     *
     * @param	cb		Callback function to be called
     * @param	us		Timeout in microseconds
     */
    void uTimerLib::setTimeout_us(void (* cb)(), unsigned long int us) {
            clearTimer();
            _cb = cb;
            _type = UTIMERLIB_TYPE_TIMEOUT;
            _attachInterrupt_us(us);
    }


    /**
     * \brief Attaches a callback function to be executed each s seconds
     *
     * @param	cb		Callback function to be called
     * @param	s		Interval in seconds
     */
    void uTimerLib::setInterval_s(void (* cb)(), unsigned long int s) {
            clearTimer();
            _cb = cb;
            _type = UTIMERLIB_TYPE_INTERVAL;
            _attachInterrupt_s(s);
    }


    /**
     * \brief Attaches a callback function to be executed once when s seconds have passed
     *
     * @param	cb		Callback function to be called
     * @param	s		Timeout in seconds
     */
    void uTimerLib::setTimeout_s(void (* cb)(), unsigned long int s) {
            clearTimer();
            _cb = cb;
            _type = UTIMERLIB_TYPE_TIMEOUT;
            _attachInterrupt_s(s);
    }

    #define UTIMERLIB_HW_COMPILE

    // Now load each hardware variation support:

    #if (defined(__AVR_ATmega32U4__) || defined(ARDUINO_ARCH_AVR)) && !defined(ARDUINO_attiny) && !defined(ARDUINO_AVR_ATTINYX4) && !defined(ARDUINO_AVR_ATTINYX5) && !defined(ARDUINO_AVR_ATTINYX7) && !defined(ARDUINO_AVR_ATTINYX8) && !defined(ARDUINO_AVR_ATTINYX61) && !defined(ARDUINO_AVR_ATTINY43) && !defined(ARDUINO_AVR_ATTINY828) && !defined(ARDUINO_AVR_ATTINY1634) && !defined(ARDUINO_AVR_ATTINYX313) && !defined(ARDUINO_AVR_DIGISPARK)
            #include "hardware/uTimerLib.AVR.cpp"
    #endif

	#if defined(ARDUINO_ARCH_AVR) && (defined(ARDUINO_attiny) || defined(ARDUINO_AVR_ATTINYX4) || defined(ARDUINO_AVR_ATTINYX5) || defined(ARDUINO_AVR_ATTINYX7) || defined(ARDUINO_AVR_ATTINYX8) || defined(ARDUINO_AVR_ATTINYX61) || defined(ARDUINO_AVR_ATTINY43) || defined(ARDUINO_AVR_ATTINY828) || defined(ARDUINO_AVR_ATTINY1634) || defined(ARDUINO_AVR_ATTINYX313))
            #include "hardware/uTimerLib.ATTINY.cpp"
    #endif

	#if defined(ARDUINO_ARCH_AVR) && defined(ARDUINO_AVR_DIGISPARK)
            #include "hardware/uTimerLib.DIGISPARK_AVR.cpp"
    #endif


    #if defined(_VARIANT_ARDUINO_STM32_) || defined(ARDUINO_ARCH_STM32)
        #include "hardware/uTimerLib.STM32.cpp"
    #endif

    #if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
            #include "hardware/uTimerLib.ESP.cpp"
    #endif
    #ifdef ARDUINO_ARCH_SAM
            #include "hardware/uTimerLib.SAM.cpp"
    #endif
    #ifdef _SAMD21_
            #include "hardware/uTimerLib.SAMD21.cpp"
    #endif
    #ifdef __SAMD51__
            #include "hardware/uTimerLib.SAMD51.cpp"
    #endif


    #if !defined(__AVR_ATmega32U4__) && !defined(ARDUINO_ARCH_AVR) && !defined(_VARIANT_ARDUINO_STM32_) && !defined(ARDUINO_ARCH_STM32) && !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP32) && !defined(ARDUINO_ARCH_SAM) && !defined(_SAMD21_) && !defined(__SAMD51__) && !defined(ARDUINO_attiny) && !defined(ARDUINO_AVR_ATTINYX5) && !defined(ARDUINO_AVR_DIGISPARK)
        #include "hardware/uTimerLib.UNSUPPORTED.cpp"
    #endif

#endif
