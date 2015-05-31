/*
 * pololu_serial.cpp
 *
 *  Created on: 21 May 2015
 *      Author: troy
 */
// Uses POSIX functions to send and receive data to the serial
// port of a Pololu Simple Motor Controller.

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/pololu_serial.h"

/**
 * Reads a variable from the SMC and returns it as number between 0 and 65535.
 * Returns SERIAL_ERROR if there was an error.
 * The 'variableId' argument must be one of IDs listed in the
 * "Controller Variables" section of the user's guide.
 * For variables that are actually signed, additional processing is required
 * (@see smcGetTargetSpeed() for an example).
 * @param fd
 * @param variableId
 * @return
*/
int smcGetVariable(int fd, unsigned char variableId)
{
  unsigned char command[] = {0xA1, variableId};
  if(write(fd, &command, sizeof(command)) == -1)
  {
//    perror("smcGetVariable: error writing");
    return SERIAL_ERROR;
  }

  unsigned char response[2];
  if(read(fd,response,2) != 2)
  {
//    perror("smcGetVariable: error reading");
    return SERIAL_ERROR;
  }

  return response[0] + 256*response[1];
}

//
//
/**
 * Returns the target speed (-3200 to 3200).
 * Returns SERIAL_ERROR if there is an error.
 * @param fd the file descriptor for the open serial port
 * @return
 */
int smcGetTargetSpeed(int fd)
{
  int val = smcGetVariable(fd, 20);
  return val == SERIAL_ERROR ? SERIAL_ERROR : (signed short)val;
}

/**
 * Get the current Simple Motor Controller error status
 * @param fd file descriptor for serial port
 * @return error bit field if successful, SERIAL_ERROR if there was an error sending
 */
int smcGetErrorStatus(int fd)
{
  return smcGetVariable(fd,0);
}

// Sends the Baud Rate auto detect command, which is required to initiate serial
// communication.
// Returns 0 if successful, SERIAL_ERROR if there was an error sending.
/**
 *
 * @param fd file descriptor for serial port
 * @return 0 if successful, SERIAL_ERROR if there was an error sending
 */
int smcAutoDetectBaudRate(int fd)
{
  const unsigned char command = 0xAA;
  if (write(fd, &command, 1) == -1)
  {
    perror("smcAutoDetectBaudRate: error writing");
    return SERIAL_ERROR;
  }
  return 0;
}

/**
 * Exit USB Safe Start mode.
 * Required to enable motor driving
 * @param fd file descriptor for serial port
 * @return 0 if successful, SERIAL_ERROR if there was an error sending
 */
int smcExitSafeStart(int fd)
{
  const unsigned char command = 0x83;
  if (write(fd, &command, 1) == -1)
  {
    perror("smcExitSafeStart: error writing");
    return SERIAL_ERROR;
  }
  return 0;
}

/**
 * Set the Simple Motor Controller target speed
 * @param fd file descriptor for serial port
 * @param speed target speed (-3200 to 3200)
 * @return 0 if successful, SERIAL_ERROR if there was an error sending
 */int smcSetTargetSpeed(int fd, int speed)
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

  if (write(fd, command, sizeof(command)) == -1)
  {
    perror("error writing");
    return SERIAL_ERROR;
  }
  return 0;
}
