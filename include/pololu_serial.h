/**
 *! @file pololu_serial.h
 *! Pololu Simple Motor Controller (SMC)
 *!
 *! @author Troy Dack
 *! @date Copyright (C) 2015
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 **/

#ifndef INCLUDE_POLOLU_SERIAL_H_
#define INCLUDE_POLOLU_SERIAL_H_

namespace Pololu {

const unsigned int SERIAL_ERROR = -9999;

const unsigned int SMC_SAFE_START_VIOLATION = (1 << 0);
const unsigned int SMC_SERIAL_ERROR		 	= (1 << 1);
const unsigned int SMC_COMMAND_TIMEOUT		= (1 << 2);
const unsigned int SMC_LIMIT_SWITCH		 	= (1 << 3);
const unsigned int SMC_LOW_VIN				= (1 << 5);
const unsigned int SMC_HIGH_VIN			 	= (1 << 6);
const unsigned int SMC_OVER_TEMP			= (1 << 7);
const unsigned int SMC_MOTOR_DRIVER_ERROR	= (1 << 8);
const unsigned int SMC_ERR_LINE_HIGH		= (1 << 9);
const int SMC_MAX_SPEED			= 3200; // Max speed controller will accept
const int SMC_MIN_SPEED			= 128;  // Min speed to move motor

/**
 *  Pololu SMC control and access class.
 *
 * Allows for easy access and control of a Pololu Simple Motor Controller.
 * Also allows read/write to all of the variables documents in the Pololu
 * SMC User's Guide.
 **/

class SMC {

private:
	int SMCfd; /**< File descriptor to the serial port */

	bool active; /**< Set to true when serial port is opened and USB
					  Safe Start has been sent */

public:
	/**
	 * Constructor.  Pass the tty device entry to be used for UART communication.
	 * Initialises comms with SMC, auto-detects baud rate and sends USB Safe start
	 * command.
	 *
	 * @param tty  path to /dev/tty of UART, eg: /dev/ttyO1
	 **/
	SMC(const char* tty);

	/**
	 * Destructor
	 **/
	~SMC();

	/**
	 * Reads a variable from the SMC and returns it as number between 0 and 65535.
	 * Returns SERIAL_ERROR if there was an error.
	 * The 'variableId' argument must be one of IDs listed in the
	 * "Controller Variables" section of the user's guide.
	 * For variables that are signed, additional processing is required
	 * @param variableId SMC variable to retrieve
	 * @return variable value or SERIAL_ERROR if there was an error
	 * @see GetTargetSpeed() for an example.
	 **/
	int GetVariable(unsigned char variableId);

	/**
	 * Returns the target speed (-3200 to 3200).
	 * @return target speed or SERIAL_ERROR if there is an error.
	 **/
	int GetTargetSpeed();

	 /**
	  * Returns a number where each bit represents a different error, and the
	  * bit is 1 if the error is currently active.
	  * See the user's guide for definitions of the different error bits.
	  * @return integer bit field of errors or SERIAL_ERROR if there is an error.
	  **/
	int GetErrorStatus();

	/** Sends the Baud Rate auto detect command, which is required to initiate serial
	 * communication.
	 * @return 0 if successful, SERIAL_ERROR if there was an error sending.
	 **/
	int AutoDetectBaudRate();

	/**
	 * Sends the Exit Safe Start command, which is required to drive the motor.
	 * @return 0 if successful, SERIAL_ERROR if there was an error sending.
	**/
	int ExitSafeStart();

	/**
	 * Sets the SMC's target speed.
	 * @param (int)speed target speed (-3200 to 3200) to send to SMC
	 * @param (float)speed target speed as a percentage (-100.0 to 100.0)
	 * @return 0 if successful, SERIAL_ERROR if there was an error sending.
	 **/
	int SetTargetSpeed(int speed);
	int SetTargetSpeed(float speed);
}; /* SMC */

} /* Pololu */

#endif /* INCLUDE_POLOLU_SERIAL_H_ */
