/**
 * uTimerLib example
 *
 * @author Naguissa
 * @url https://www.github.com/Naguissa/uTimerLib
 * @url https://www.foroelectro.net
 */

#include "Arduino.h"
#include "uTimerLib.h"

#ifndef LED_BUILTIN
	// change to fit your needs
	// Use 0 or 1 to use DigiSpark AVR internal LED (depending revision, mine is 1)
	#define LED_BUILTIN 13
#endif

volatile bool status = 0;

void timed_function() {
	status = !status;
	if (status) {
		TimerLib.setTimeout_s(timed_function, 2);
	}
}


void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	TimerLib.setTimeout_s(timed_function, 2);
}


void loop() {
	digitalWrite(LED_BUILTIN, status);
}

