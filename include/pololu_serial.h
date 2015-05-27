/*
 * pololu_serial.h
 *
 *  Created on: 21 May 2015
 *      Author: troy
 */

#ifndef INCLUDE_POLOLU_SERIAL_H_
#define INCLUDE_POLOLU_SERIAL_H_

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define SERIAL_ERROR -9999

// Reads a variable from the SMC and returns it as number between 0 and 65535.
// Returns SERIAL_ERROR if there was an error.
// The 'variableId' argument must be one of IDs listed in the
// "Controller Variables" section of the user's guide.
// For variables that are actually signed, additional processing is required
// (see smcGetTargetSpeed for an example).
int smcGetVariable(int fd, unsigned char variableId);

// Returns the target speed (-3200 to 3200).
// Returns SERIAL_ERROR if there is an error.
int smcGetTargetSpeed(int fd);

// Returns a number where each bit represents a different error, and the
// bit is 1 if the error is currently active.
// See the user's guide for definitions of the different error bits.
// Returns SERIAL_ERROR if there is an error.
int smcGetErrorStatus(int fd);

// Sends the Baud Rate auto detect command, which is required to initiate serial
// communication.
// Returns 0 if successful, SERIAL_ERROR if there was an error sending.
int smcAutoDetectBaudRate(int fd);

// Sends the Exit Safe Start command, which is required to drive the motor.
// Returns 0 if successful, SERIAL_ERROR if there was an error sending.
int smcExitSafeStart(int fd);

// Sets the SMC's target speed (-3200 to 3200).
// Returns 0 if successful, SERIAL_ERROR if there was an error sending.
int smcSetTargetSpeed(int fd, int speed);


#endif /* INCLUDE_POLOLU_SERIAL_H_ */
