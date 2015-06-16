/**
 *! @file pololu_serial.cpp
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

#include <fcntl.h>
#include <pololu_serial.h>
#include <termios.h> // POSIX terminal control definitions
#include <unistd.h>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace Pololu {

	SMC::SMC(const char* tty) {
		struct termios options;

		SMCfd = open(tty, O_RDWR | O_NOCTTY | O_NDELAY);
		if (SMC::SMCfd == -1) {
			throw std::runtime_error("Unable to open " + std::string(tty));
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
			return;
		}
	}

	SMC::~SMC() {
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
	int SMC::SetTargetSpeed(float speed) {
		return SetTargetSpeed( (int)(SMC_MAX_SPEED * speed/100) );
	}
} /* Pololu */
