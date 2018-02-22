/**
 * uTimerLib example
 *
 * @author Naguissa
 * @url https://www.github.com/Naguissa/uTimerLib
 * @url https://www.foroelectro.net
 */

#include "Arduino.h"
#include "uTimerLib.h"



volatile bool status = 0;

void timed_function() {
	status = !status;
	digitalWrite(LED_BUILTIN, status);
}


void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	TimerLib.setInterval_s(timed_function, 2);
}


void loop() {
}

