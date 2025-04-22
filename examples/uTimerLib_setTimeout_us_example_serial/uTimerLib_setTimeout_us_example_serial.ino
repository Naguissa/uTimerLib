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
volatile unsigned long int actMilis = 0;
volatile bool trigger = false;

void timed_function() {
	trigger = true;
}

void setup() {
	Serial.begin(57600);
	TimerLib.setTimeout_us(timed_function, 200000);
	prevMillis = millis();
}

void loop() {
	if (trigger) {
		trigger = false;
		Serial.println(actMillis - prevMillis);
		prevMillis = actMillis;
	}
	actMillis = millis();
}

