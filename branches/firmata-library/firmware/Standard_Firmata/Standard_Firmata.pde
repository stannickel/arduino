/*
 * Copyright (C) 2006-2008 Free Software Foundation.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE.txt for further informations on licensing terms.
 */

/* 
 * TODO: generalized SysEx support
 * TODO: Bjoen Servo support
 * TODO: Firmata.read() callback registry
 * TODO: pulseOut functionality for servos
 * TODO: software PWM for servos, etc (servo.h or pulse.h)
 * TODO: device type reporting (i.e. some firmwares will use the Firmata
 *       protocol, but will only support specific devices, like ultrasound 
 *       rangefinders or servos)
 * TODO: use Program Control to load stored profiles from EEPROM
 */

#include <EEPROM.h>
#include <Firmata.h>

/*==============================================================================
 * GLOBAL VARIABLES
 *============================================================================*/

/* analog inputs */
int analogInputsToReport = 0; // bitwise array to store pin reporting
int analogPin = 0; // counter for reading analog pins
/* digital pins */
boolean digitalInputsEnabled = false; // output digital inputs or not
int digitalInputs;
int previousDigitalInputs; // previous output to test for change
int digitalPinStatus = 65535; // store pin status, default OUTPUT
/* PWM/analog outputs */
int pwmStatus = 0; // bitwise array to store PWM status, default off
/* timer variables */
extern volatile unsigned long timer0_overflow_count; // timer0 from wiring.c
unsigned long nextExecuteTime; // for comparison with timer0_overflow_count

/*==============================================================================
 * FUNCTIONS                                                                
 *============================================================================*/

/* -----------------------------------------------------------------------------
 * check all the active digital inputs for change of state, then add any events
 * to the Serial output queue using Serial.print() */
void checkDigitalInputs(void) 
{
    if(digitalInputsEnabled) {
        // TODO reimplement report_digital_pins
        // TODO perhaps ignore pins 0,1 and 14,15 in checking against previous
        previousDigitalInputs = digitalInputs;
        digitalInputs = PINB << 8;  // get pins 8-13
        digitalInputs += PIND;      // get pins 0-7
        if(digitalInputs != previousDigitalInputs) {
            Firmata.sendDigitalPortPair(0, digitalInputs); // Arduino pins are in port 0
        }
    }
}

// -----------------------------------------------------------------------------
/* sets the pin mode to the correct state and sets the relevant bits in the
 * two bit-arrays that track Digital I/O and PWM status
 */
void setPinMode(byte pin, int mode) {
  if(pin > 1) { // ignore RxTx pins (0,1)
	if(mode == INPUT) {
	  digitalPinStatus = digitalPinStatus &~ (1 << pin);
	  pwmStatus = pwmStatus &~ (1 << pin);
	  digitalWrite(pin,LOW); // turn off pin before switching to INPUT
	  pinMode(pin,INPUT);
	}
	else if(mode == OUTPUT) {
	  digitalPinStatus = digitalPinStatus | (1 << pin);
	  pwmStatus = pwmStatus &~ (1 << pin);
	  pinMode(pin,OUTPUT);
	}
	else if( mode == PWM ) {
	  digitalPinStatus = digitalPinStatus | (1 << pin);
	  pwmStatus = pwmStatus | (1 << pin);
	  pinMode(pin,OUTPUT);
	}
  // TODO: save status to EEPROM here, if changed
  }
}

void analogWriteCallback(byte pin, int value)
{
    setPinMode(pin,PWM);
    analogWrite(pin, value);
}

void digitalWriteCallback(byte port, int value)
{
/* TODO should digitalPinStatus still be used?  it is useful to be able to
 * toggle the pin when in INPUT mode in order to control the internal pull-up
 * resistor.  Perhaps there needs to be some safety on that tho.
 */
    
// pins 2-7  (0,1 are for the serial RX/TX, don't change their values)
// 0xFF03 == B1111111100000011    0x03 == B00000011
    PORTB = (value &~ 0xFF03) | (PORTB &~ 0x03);
    
//pins 8-13 (14,15 are for the crystal, don't change their values)
// 0xffc0 == B1111111111000000    0xc0 == B11000000
    PORTD = (value >> 8 &~ 0xFFC0) | (PORTD &~ 0xC0);   
}

// -----------------------------------------------------------------------------
/* sets bits in a bit array (int) to toggle the reporting of the analogIns
 */
//void FirmataClass::setAnalogPinReporting(byte pin, byte state) {
//}
void reportAnalogCallback(byte pin, int value)
{
    if(value == 0) {
        analogInputsToReport = analogInputsToReport &~ (1 << pin);
    }
    else { // everything but 0 enables reporting of that pin
        analogInputsToReport = analogInputsToReport | (1 << pin);
    }
    // TODO: save status to EEPROM here, if changed
}

void reportDigitalCallback(byte port, int value)
{
// TODO: implement MIDI channel as port base for more than 16 digital inputs
    if(value == 0)
        digitalInputsEnabled = false;
    else
        digitalInputsEnabled = true;
}

/*==============================================================================
 * SETUP()
 *============================================================================*/
void setup() 
{
    byte i;

    Firmata.attach(ANALOG_MESSAGE, analogWriteCallback);
    Firmata.attach(DIGITAL_MESSAGE, digitalWriteCallback);
    Firmata.attach(REPORT_ANALOG, reportAnalogCallback);
    Firmata.attach(REPORT_DIGITAL, reportDigitalCallback);
    Firmata.attach(SET_PIN_MODE, setPinMode);

    for(i=0; i<TOTAL_DIGITAL_PINS; ++i) {
        setPinMode(i,OUTPUT);
    }
    // TODO: load state from EEPROM here

    /* TODO: send digital inputs here, if enabled, to set the initial state on the
     * host computer, since once in the loop(), the Arduino will only send data on
     * change. */

    Firmata.begin();
}

/*==============================================================================
 * LOOP()
 *============================================================================*/
void loop() 
{
/* DIGITALREAD - as fast as possible, check for changes and output them to the
 * FTDI buffer using Serial.print()  */
    checkDigitalInputs();  
    if(timer0_overflow_count > nextExecuteTime) {  
        nextExecuteTime = timer0_overflow_count + 19; // run this every 20ms
        /* SERIALREAD - Serial.read() uses a 128 byte circular buffer, so handle
         * all serialReads at once, i.e. empty the buffer */
        while(Firmata.available())
            Firmata.processInput();
        /* SEND FTDI WRITE BUFFER - make sure that the FTDI buffer doesn't go over
         * 60 bytes. use a timer to sending an event character every 4 ms to
         * trigger the buffer to dump. */
	
        /* ANALOGREAD - right after the event character, do all of the
         * analogReads().  These only need to be done every 4ms. */
        for(analogPin=0;analogPin<TOTAL_ANALOG_PINS;analogPin++) {
            if( analogInputsToReport & (1 << analogPin) ) 
                Firmata.sendAnalog(analogPin, analogRead(analogPin));
        }
    }
}
