/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to ESP32 pin 12. ( set at line 262 of IRremote.h)
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * This sketch sends the power command to a Sony TV
 * Adapted by others along the way including Gareth Coleman
 */


#include <IRremote.h>

IRsend irsend;

void setup()
{
}

void loop() {
	for (int i = 0; i < 3; i++) {     // Sony protocol requires three repeats of each command
		irsend.sendSony(0xa90, 12);     // 0xa90 combines devices ID 1 for TV with code 21 for power.
		                                // Format is 2nd param, for Sony either 12 or 20 (mostly).
    //irsend.sendNEC(value, bits);  // other formats have helper funcions too - 
    //irsend.sendRC5(value, bits);  // look at the examples for the IRremote library to get syntax.

		delay(40);
	}
	delay(5000); //5 second delay between each signal burst
}
