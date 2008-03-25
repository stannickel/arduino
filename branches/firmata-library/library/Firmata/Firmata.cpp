/*
  Firmata.cpp - Firmata library
  Copyright (c) 2007-2008 Free Software Foundation.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//******************************************************************************
//* Includes
//******************************************************************************

extern "C" {
    // AVR LibC Includes
#include <inttypes.h>
#include <stdlib.h>

    // use the abs in WConstants.h, not the one in stdlib.h
#undef abs
  
    // Wiring Core Includes
#include "WConstants.h"
}


#include "Firmata.h"
#include "EEPROM.h"
#include "HardwareSerial.h"


//******************************************************************************
//* Definitions
//******************************************************************************

//******************************************************************************
//* Constructors
//******************************************************************************

FirmataClass::FirmataClass()
{
    byte i;
	// TODO: init serial here
	// TODO: printVersion
    analogReceiveFunctionCount = 0;
    waitForData = 0; // this flag says the next serial input will be data
    executeMultiByteCommand = 0; // execute this after getting multi-byte data
    multiByteChannel = 0; // channel data for multiByteCommands
    for(i=0; i<MAX_DATA_BYTES; i++) {
        storedInputData[i] = 0;
    }

    digitalPinStatus = 65535; // store pin status, default OUTPUT
/* PWM/analog outputs */
    pwmStatus = 0; // bitwise array to store PWM status

  for(i=0; i<TOTAL_DIGITAL_PINS; ++i) {
    setPinMode(i,OUTPUT);
  }
}

//******************************************************************************
//* Public Methods
//******************************************************************************

/* begin method for overriding default serial bitrate */
void FirmataClass::begin()
{
    blinkVersion();
    Serial.begin(115200);
    delay(300);
    printVersion();
}

/* begin method for overriding default serial bitrate */
void FirmataClass::begin(long speed)
{
    blinkVersion();
    Serial.begin(speed);
    delay(300);
    printVersion();
}

// output the protocol version message to the serial port
void FirmataClass::printVersion() {
    Serial.print(REPORT_VERSION, BYTE);
    Serial.print(FIRMATA_MINOR_VERSION, BYTE);
    Serial.print(FIRMATA_MAJOR_VERSION, BYTE);
}

//------------------------------------------------------------------------------
// Serial Receive Handling

int FirmataClass::available(void)
{
    return Serial.available();
}

void FirmataClass::processInput(void)
{
    int inputData = Serial.read(); // this is 'int' to handle -1 when no data
    int command;
    
    // a few commands have byte(s) of data following the command
    if( (waitForData > 0) && (inputData < 128) ) {  
        waitForData--;
        storedInputData[waitForData] = inputData;
        if( (waitForData==0) && executeMultiByteCommand ) { // got the whole message
            switch(executeMultiByteCommand) {
            case ANALOG_MESSAGE:
                setPinMode(multiByteChannel,PWM);
                analogWrite(multiByteChannel, 
                            (storedInputData[0] << 7) + storedInputData[1] );
                break;
            case DIGITAL_MESSAGE:
                outputDigitalBytes(storedInputData[1], storedInputData[0]); //(LSB, MSB)
                break;
            case REPORT_ANALOG_PIN:
                setAnalogPinReporting(multiByteChannel,storedInputData[0]);
                break;
            }
            executeMultiByteCommand = 0;
        }	
    } else {
        // remove channel info from command byte if less than 0xF0
        if(inputData < 0xF0) {
            command = inputData & 0xF0;
            multiByteChannel = inputData & 0x0F;
        } else {
            command = inputData;
            // commands in the 0xF* range don't use channel data
        }
        switch (command) { // TODO: these needs to be switched to command
        case ANALOG_MESSAGE:
        case DIGITAL_MESSAGE:
        case SET_DIGITAL_PIN_MODE:
            waitForData = 2; // two data bytes needed
            executeMultiByteCommand = command;
            break;
        case REPORT_ANALOG_PIN:
        case REPORT_DIGITAL_PORTS:
            waitForData = 1; // two data bytes needed
            executeMultiByteCommand = command;
            break;
        case SYSTEM_RESET:
            // this doesn't do anything yet
            break;
        case REPORT_VERSION:
            Firmata.printVersion();
            break;
        }
    }
}



//------------------------------------------------------------------------------
// Serial Send Handling

// send an analog message
void FirmataClass::sendAnalog(int pin, int value) 
{
	// pin can only be 0-15, so chop higher bits
	Serial.print(ANALOG_MESSAGE | (pin & 0xF), BYTE);
	Serial.print(value % 128, BYTE);
	Serial.print(value >> 7, BYTE); 
}

// send a single digital pin in a digital message
void FirmataClass::sendDigital(int pin, int value) 
{
	// TODO add single pin digital messages to the  protocol
}

// send 14-bits in a single digital message
void FirmataClass::sendDigitalPortPair(int port, int value) 
{
	// TODO: the digital message should not be sent on the serial port every
	// time sendDigital() is called.  Instead, it should add it to an int
	// which will be sent on a schedule.  If a pin changes more than once
	// before the digital message is sent on the serial port, it should send a
	// digital message for each change.
 
	// TODO: some math needs to happen for pin > 14 since MIDI channels are used
	Serial.print(DIGITAL_MESSAGE | (port & 0xF),BYTE);
	Serial.print(value % 128, BYTE); // Tx pins 0-6
	Serial.print(value >> 7, BYTE);  // Tx pins 7-13
}


// send a single digital pin in a digital message
void FirmataClass::sendSysex(byte command, byte bytec, byte* bytev) 
{
    byte i;
    Serial.print(START_SYSEX, BYTE);
    Serial.print(command, BYTE);
    for(i=0; i<bytec; i++) {
        Serial.print(bytev[i], BYTE);        
    }
    Serial.print(END_SYSEX, BYTE);
}


// Internal Actions/////////////////////////////////////////////////////////////

void FirmataClass::loadState(void)
{
	// TODO load state from EEPROM
}

void FirmataClass::saveState(void)
{
	// TODO save state to EEPROM
}

void FirmataClass::resetState(void)
{
	// TODO reset state bytes in EEPROM
}

void FirmataClass::attachAnalogReceive(int pin, analogReceiveFunction newFunction)
{
    byte i;
    byte tmpCount = analogReceiveFunctionCount;
    analogReceiveFunction* tmpArray = analogReceiveFunctionArray;
    analogReceiveFunctionCount++;
    analogReceiveFunctionArray = (analogReceiveFunction*) calloc(analogReceiveFunctionCount, sizeof(analogReceiveFunction));
    for(i = 0; i < tmpCount; i++) {
        analogReceiveFunctionArray[i] = tmpArray[i];
    }
    analogReceiveFunctionArray[tmpCount] = newFunction;
}


//******************************************************************************
//* Private Methods
//******************************************************************************




/* -----------------------------------------------------------------------------
 * output digital bytes received from the serial port  */
void FirmataClass::outputDigitalBytes(byte pin0_6, byte pin7_13) {
  int i;
  int mask;
  int twoBytesForPorts;
    
// this should be converted to use PORTs
  twoBytesForPorts = pin0_6 + (pin7_13 << 7);
  for(i=2; i<TOTAL_DIGITAL_PINS; ++i) { // ignore Rx,Tx pins (0 and 1)
    mask = 1 << i;
    if( (digitalPinStatus & mask) && !(pwmStatus & mask) ) {
      digitalWrite(i, twoBytesForPorts & mask ? HIGH : LOW);
    } 
  }
}

// -----------------------------------------------------------------------------
/* sets the pin mode to the correct state and sets the relevant bits in the
 * two bit-arrays that track Digital I/O and PWM status
 */
void FirmataClass::setPinMode(byte pin, byte mode) {
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

// -----------------------------------------------------------------------------
/* sets bits in a bit array (int) to toggle the reporting of the analogIns
 */
void FirmataClass::setAnalogPinReporting(byte pin, byte state) {
    /*
    if(state == 0) {
        analogInputsToReport = analogInputsToReport &~ (1 << pin);
    }
    else { // everything but 0 enables reporting of that pin
        analogInputsToReport = analogInputsToReport | (1 << pin);
    }
    */
    // TODO: save status to EEPROM here, if changed
}



// resets the system state upon a SYSTEM_RESET message from the host software
void FirmataClass::systemReset(void)
{
    // TODO automatically call this in response to SYSTEM_RESET
    // TODO reset EEPROM to 0 here
    // TODO empty serial buffer here
}



// =============================================================================
// used for flashing the pin for the version number
void FirmataClass::pin13strobe(int count, int onInterval, int offInterval) 
{
    byte i;
    pinMode(VERSION_BLINK_PIN, OUTPUT);
    for(i=0; i<count; i++) {
        delay(offInterval);
        digitalWrite(VERSION_BLINK_PIN, HIGH);
        delay(onInterval);
        digitalWrite(VERSION_BLINK_PIN, LOW);
    }
}

void FirmataClass::blinkVersion(void)
{
    // flash the pin with the protocol version
    pinMode(VERSION_BLINK_PIN,OUTPUT);
    pin13strobe(2,1,4); // separator, a quick burst
    delay(500);
    pin13strobe(FIRMATA_MAJOR_VERSION, 200, 400);
    delay(500);
    pin13strobe(2,1,4); // separator, a quick burst
    delay(500);
    pin13strobe(FIRMATA_MINOR_VERSION, 200, 400);
    delay(500);
    pin13strobe(2,1,4); // separator, a quick burst
}

// make one instance for the user to use
FirmataClass Firmata;
