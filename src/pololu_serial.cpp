/**
 * This library provides an interface to a Pololu Simple Motor Controller (SMC)
 *
 *  Created on: 21 May 2015
 *      Author: Troy Dack
**/
// Uses POSIX functions to send and receive data to the serial
// port of a Pololu Simple Motor Controller.

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h> // POSIX terminal control definitions
#include <iostream>
#include "pololu_serial.h"

namespace Pololu {

	SMC::SMC(const char* __tty) {
		struct termios options;

		SMCfd = open(__tty, O_RDWR | O_NOCTTY | O_NDELAY);
		if (SMC::SMCfd == -1) {
			std::cerr << "Unable to open " << __tty << std::endl;
			active = false;
		} else {
			fcntl(SMCfd, F_SETFL, FNDELAY);
			tcgetattr(SMCfd, &options);
			cfsetispeed(&options, B115200);
			cfsetospeed(&options, B115200);
			options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
			options.c_oflag &= ~(ONLCR | OCRNL);
			tcsetattr(SMCfd, TCSANOW, &options);
			// Initialise comms with motor controller
			AutoDetectBaudRate();
			// Exit USB safe start
			ExitSafeStart();
			active = true;
			return;
		}
	}

	SMC::~SMC() {
		active = false;
		close(SMCfd);
	}

	int SMC::GetVariable(unsigned char variableId)
	{
	  unsigned char command[] = {0xA1, variableId};
	  if(write(SMCfd, &command, sizeof(command)) == -1)
	  {
	    perror("smcGetVariable: error writing");
		return SERIAL_ERROR;
	  }

	  unsigned char response[2];
	  if(read(SMCfd,response,2) != 2)
	  {
	    perror("smcGetVariable: error reading");
		return SERIAL_ERROR;
	  }

	  return response[0] + 256*response[1];
	}

	int SMC::GetTargetSpeed()
	{
	  int val = GetVariable(20);
	  return val == SERIAL_ERROR ? SERIAL_ERROR : (signed short)val;
	}

	int SMC::GetErrorStatus()
	{
	  return GetVariable(0);
	}

	int SMC::AutoDetectBaudRate()
	{
	  const unsigned char command = 0xAA;
	  if (write(SMCfd, &command, 1) == -1)
	  {
		perror("SMC::AutoDetectBaudRate: error writing");
		return SERIAL_ERROR;
	  }
	  return 0;
	}

	int SMC::ExitSafeStart()
	{
	  const unsigned char command = 0x83;
	  if (write(SMCfd, &command, 1) == -1)
	  {
		perror("SMC::ExitSafeStart: error writing");
		return SERIAL_ERROR;
	  }
	  return 0;
	}

	int SMC::SetTargetSpeed(int speed)
	{
	  unsigned char command[3];

	  if (speed < 0)
	  {
		command[0] = 0x86; // Motor Reverse
		speed = -speed;
	  }
	  else
	  {
		command[0] = 0x85; // Motor Forward
	  }
	  command[1] = speed & 0x1F;
	  command[2] = speed >> 5 & 0x7F;

	  if (write(SMCfd, command, sizeof(command)) == -1)
	  {
		perror("error writing");
		return SERIAL_ERROR;
	  }
	  return 0;
	}

} /* Pololu */
