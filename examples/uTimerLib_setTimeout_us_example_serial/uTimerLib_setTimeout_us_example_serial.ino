/**
 * uTimerLib example
 *
 * @author Naguissa
 * @url https://www.github.com/Naguissa/uTimerLib
 * @url https://www.foroelectro.net
 */

#include "Arduino.h"
#include "uTimerLib.h"



unsigned long int prevMillis = 0;

void timed_function() {
	Serial.println(millis() - prevMillis);
}



void setup() {
	Serial.begin(57600);
	TimerLib.setTimeout_us(timed_function, 200000);
	prevMillis = millis();
}


void loop() {
}

