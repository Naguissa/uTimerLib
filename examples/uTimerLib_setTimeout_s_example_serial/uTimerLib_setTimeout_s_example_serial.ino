/**
 * uTimerLib example
 *
 * @author Naguissa
 * @url https://www.github.com/Naguissa/uTimerLib
 * @url https://www.foroelectro.net
 */

#include "Arduino.h"
#include "uTimerLib.h"

volatile unsigned long int prevMillis = 0;
volatile unsigned long int actMillis = 0;

void timed_function() {
	Serial.println(actMillis - prevMillis);
}

void setup() {
	Serial.begin(57600);
	TimerLib.setTimeout_s(timed_function, 2);
	prevMillis = millis();
}

void loop() {
	actMillis = millis();
}
