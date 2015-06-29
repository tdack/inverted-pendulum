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
#include <iostream>

namespace Pololu {

	SMC::SMC(const char* tty) {
		struct termios options;

		SMCfd = open(tty, O_RDWR | O_NOCTTY | O_NDELAY);
		ttyActive = false;
		if (SMC::SMCfd == -1) {
			throw std::runtime_error("Unable to open " + std::string(tty));
		} else {
			fcntl(SMCfd, F_SETFL, FNDELAY);
			tcgetattr(SMCfd, &options);
			cfsetispeed(&options, B19200);
			cfsetospeed(&options, B19200);
			options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
			options.c_oflag &= ~(ONLCR | OCRNL);
			tcsetattr(SMCfd, TCSANOW, &options);
			AutoDetectBaudRate(); // Initialise comms with motor controller
			ExitSafeStart(); // Exit USB safe start
			SetTargetSpeed(0); // Set speed to 0 initially just to be safe.
			return;
		}
	}

	SMC::~SMC() {
		close(SMCfd);
	}

	int SMC::serial_write(const unsigned char *buffer, int len) {
		if (ttyActive.load()) {
			std::cout << "tty busy" << std::endl;
			return -1;
		}
		int bytes_written = 0;
		bytes_written = write(SMCfd, buffer, len);
		if (bytes_written == -1) {
			perror("Couldn't write data");
		}
		ttyActive.store(false);
		return bytes_written;
	}

	int SMC::serial_read() {
		unsigned char response[2];
		if (ttyActive.load()) {
			return -1;
		} else {
			ttyActive.store(true);
		}

		if(read(SMCfd,response,2) != 2)
		{
			perror("smcGetVariable: error reading");
			ttyActive.store(false);
			return SERIAL_ERROR;
		}
		ttyActive.store(false);
		return response[0] + 256 * response[1];
	}

	int SMC::GetVariable(unsigned char variableId)
	{
	  unsigned char command[] = {0xA1, variableId};
	  serial_write(command, sizeof(command));
	  return serial_read();
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
	  unsigned char command[] = {0xAA}; // Autodetect baud rate command
	  serial_write(command, sizeof command);
	  return 0;
	}

	int SMC::ExitSafeStart()
	{
	  unsigned char command[] = {0x83}; // Exit safe start command
	  serial_write(command, sizeof command);
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

	  serial_write(command, sizeof command);

	  return 0;
	}

	int SMC::SetTargetSpeed(float speed) {
		return SetTargetSpeed( (int)(SMC_MAX_SPEED * speed/100) );
	}
} /* Pololu */
