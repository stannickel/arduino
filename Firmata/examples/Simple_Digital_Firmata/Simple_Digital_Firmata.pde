/* Copyright (C) 2008 Free Software Foundation.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE.txt for further informations on licensing terms.
 */

/* Supports as many digital inputs and outputs as possible.
 *
 * Written by Hans-Christoph Steiner <hans@eds.org>
 */
#include <Firmata.h>

/* digital pins */
byte reportPINs[TOTAL_PORTS];
byte previousPINs[TOTAL_PORTS];
byte previousPORTs[TOTAL_PORTS];

extern volatile unsigned long timer0_overflow_count; // timer0 from wiring.c
unsigned long nextExecuteTime; // for comparison with timer0_overflow_count

byte whichOutput = 0;

void outputPort(byte portNumber, byte portValue)
{
    whichOutput = ++whichOutput % 128;
    if(previousPINs[portNumber] != portValue) {
        Firmata.sendDigitalPort(portNumber, portValue); 
        previousPINs[portNumber] = portValue;
        Firmata.sendDigitalPort(portNumber, portValue); 
    }
}

void checkDigitalInputs(void) 
{
    byte i, tmp;
    for(i=0; i < TOTAL_PORTS; i++) {
        if(reportPINs[i]) {
            switch(i) {
            case 0: outputPort(0, PIND); break;
            case 1: outputPort(1, PINB); break;
            case ANALOG_PORT: outputPort(ANALOG_PORT, PINC); break;
            }
        }
    }
}

void setPinModeCallback(byte pin, int mode) {
    if(pin > 1) { // ignore RxTx pins (0,1)
        pinMode(pin, mode);
    }
}

void digitalWriteCallback(byte port, int value)
{
    byte i;
    byte currentPinValue, previousPinValue;

    if(value != previousPORTs[port]) {
        for(i=0; i<8; i++) {
            currentPinValue = (byte) value & (1 << i);
            previousPinValue = previousPORTs[port] & (1 << i);
            if(currentPinValue != previousPinValue) {
                digitalWrite(i + (port*8), currentPinValue);
            }
        }
        previousPORTs[port] = value;
    }
}

void reportDigitalCallback(byte port, int value)
{
    reportPINs[port] = (byte)value;
}

void setup()
{
    byte i;
    
    Firmata.setFirmwareVersion(0, 1);
    Firmata.attach(DIGITAL_MESSAGE, digitalWriteCallback);
    Firmata.attach(SET_PIN_MODE, setPinModeCallback);
    Firmata.attach(REPORT_DIGITAL, reportDigitalCallback);
    
    for(i=2; i<TOTAL_DIGITAL_PINS; ++i) { // ignore RxTx pins (0,1)
        pinMode(i,OUTPUT);
    }
    for(i=0; i<TOTAL_PORTS; ++i) {
        reportPINs[i] = true;
    }
    Firmata.begin();
}

void loop()
{
    if(timer0_overflow_count > nextExecuteTime) {  
        nextExecuteTime = timer0_overflow_count + 19; // run this every 20ms
        checkDigitalInputs();
        while(Firmata.available()) {
            Firmata.processInput();
        }
    }
}
