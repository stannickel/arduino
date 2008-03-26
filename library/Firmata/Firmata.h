/*
  Firmata.h - Firmata library
  Copyright (c) 2007-2008 Free Software Foundation.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef Firmata_h
#define Firmata_h

#include <inttypes.h>


/* Version numbers for the protocol.  The protocol is still changing, so these
 * version numbers are important.  This number can be queried so that host
 * software can test whether it will be compatible with the currently
 * installed firmware. */
#define FIRMATA_MAJOR_VERSION   1 // for non-compatible changes
#define FIRMATA_MINOR_VERSION   0 // for backwards compatible changes

#define MAX_DATA_BYTES 32 // max number of data bytes in non-Sysex messages

extern "C" {
// callback function types
    typedef void (*callbackFunction)(byte, int);
//    typedef void (*sysexReceiveFunction)(byte, byte, byte*);
}

class FirmataClass
{
public:
	FirmataClass();

//----------------------------------------
// methods
// Arduino constructors
    void begin();
    void begin(long);
// querying functions
	void printVersion(void);
    void blinkVersion(void);
// serial receive handling
    int available(void);
    void processInput(void);
// serial send handling
	void sendAnalog(int, int);
	void sendDigital(int, int);
	void sendDigitalPortPair(int, int);
	void sendSysex(byte, byte, byte*);
// attach & detach functions to messages
// TODO make general attach & detach functions
    void attachAnalogReceive(callbackFunction);
    void detachAnalogReceive(void);
    void attachDigitalReceive(callbackFunction);
    void detachDigitalReceive(void);
    void attachReportAnalog(callbackFunction);
    void detachReportAnalog(void);
    void attachReportDigital(callbackFunction);
    void detachReportDigital(void);
    void attachPinMode(callbackFunction);
    void detachPinMode(void);
private:
/* input message handling */
    byte waitForData; // this flag says the next serial input will be data
    byte executeMultiByteCommand; // execute this after getting multi-byte data
    byte multiByteChannel; // channel data for multiByteCommands
    byte storedInputData[MAX_DATA_BYTES]; // multi-byte data
/* argc/argv pairs for callback functions */
    callbackFunction currentAnalogCallback;
    callbackFunction currentDigitalCallback;
    callbackFunction currentReportAnalogCallback;
    callbackFunction currentReportDigitalCallback;
    callbackFunction currentPinModeCallback;
//    byte sysexReceiveFunctionCount;
//    sysexReceiveFunction* sysexReceiveFunctionArray;

//----------------------------------------
// private methods
    void outputDigitalBytes(byte pin0_6, byte pin7_13);
    void setAnalogPinReporting(byte pin, byte state);
	void systemReset(void);
    void pin13strobe(int count, int onInterval, int offInterval);
};

extern FirmataClass Firmata;

/*==============================================================================
 * MACROS
 *============================================================================*/

// total number of pins currently supported
#if defined(__AVR_ATmega168__) // Arduino NG and Diecimila
#define TOTAL_ANALOG_PINS       8
#define TOTAL_DIGITAL_PINS      14
#elif defined(__AVR_ATmega8__)  // old Arduinos
#define TOTAL_ANALOG_PINS       6
#define TOTAL_DIGITAL_PINS      14
#elif defined(__AVR_ATmega128__)  // Wiring
#define TOTAL_ANALOG_PINS       8
#define TOTAL_DIGITAL_PINS      43
#endif


// for comparing along with INPUT and OUTPUT
#define PWM                     2

// for selecting digital inputs
#define PB  2  // digital input, pins 8-13
#define PC  3  // analog input port
#define PD  4  // digital input, pins 0-7

// message command bytes
#define DIGITAL_MESSAGE         0x90 // send data for a digital pin
#define ANALOG_MESSAGE          0xE0 // send data for an analog pin (or PWM)
#define REPORT_ANALOG_PIN       0xC0 // enable analog input by pin #
#define REPORT_DIGITAL_PORTS    0xD0 // enable digital input by port pair
//
#define SET_PIN_MODE            0xF4 // set a pin to INPUT/OUTPUT/PWM/etc
//
#define START_SYSEX             0xF0 // start a MIDI Sysex message
#define END_SYSEX               0xF7 // end a MIDI Sysex message
//
#define REPORT_VERSION          0xF9 // report firmware version
#define SYSTEM_RESET            0xFF // reset from MIDI

// these are used for EEPROM reading and writing
#define ANALOGINPUTSTOREPORT_LOW_BYTE   0x1F0 // analogInputsToReport is an int
#define ANALOGINPUTSTOREPORT_HIGH_BYTE  0x1F1 // analogInputsToReport is an int
#define REPORTDIGITALINPUTS_BYTE        0x1F2 // 
#define DIGITALPINSTATUS_LOW_BYTE       0x1F3 // digitalPinStatus is an int
#define DIGITALPINSTATUS_HIGH_BYTE      0x1F4 // digitalPinStatus is an int
#define PWMSTATUS_LOW_BYTE              0x1F5 // pwmStatus is an int
#define PWMSTATUS_HIGH_BYTE             0x1F6 // pwmStatus is an int

#define VERSION_BLINK_PIN               13 // digital pin to blink version on


#endif /* Firmata_h */

