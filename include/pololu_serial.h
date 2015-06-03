/*
 * pololu_serial.h
 *
 *  Created on: 21 May 2015
 *      Author: troy
 */
/*! \file pololu_serial.h
 * \brief Pololu SMC header file.
 * Contains all of the method declarations for class SMC.
**/

#ifndef INCLUDE_POLOLU_SERIAL_H_
#define INCLUDE_POLOLU_SERIAL_H_

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>


namespace Pololu {
#define SERIAL_ERROR -9999
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
	 * @param __tty  path to /dev/tty of UART, eg: /dev/ttyO1
	 **/
	SMC(const char* __tty);

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
	 * @param speed target speed (-3200 to 3200) to send to SMC
	 * @return 0 if successful, SERIAL_ERROR if there was an error sending.
	 **/
	int SetTargetSpeed(int speed);
}; /* SMC */

} /* Pololu */

#endif /* INCLUDE_POLOLU_SERIAL_H_ */
