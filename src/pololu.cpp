/*
 * pololu.cpp
 *
 *  Created on: 21 May 2015
 *      Author: troy
 */

#include <unistd.h>  // UNIX standard function definitions
#include <fcntl.h>   // file control definitions
#include <iostream>  // Input-Output streams
#include <termios.h> // POSIX terminal control definitions
#include <sys/time.h>
#include <sys/stat.h>

// Language dependencies
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>

#include "../include/pololu.h"
#include "../include/bbb-eqep.h"
#include "../include/pololu_serial.h"
#include "../include/debug.h"

#define DEBUG_LEVEL 1

using std::cout;
using std::endl;
using std::cerr;

int open_port(void){
    int fd;
    struct termios options;

    fd = open(POLOLU_TTY, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        std::cerr << "Unable to open " << POLOLU_TTY << std::endl;
    } else {
        fcntl(fd, F_SETFL, FNDELAY);
        tcgetattr(fd, &options);
        cfsetispeed(&options, B115200);
        cfsetospeed(&options, B115200);
        options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        options.c_oflag &= ~(ONLCR | OCRNL);
        tcsetattr(fd, TCSANOW, &options);
    }
    return (fd);
}

int read_eQEP ()
{
  struct timeval tv1, tv2;
  int eqep_num = 0;
  uint32_t eqep_pos;

  BBB::eQEP eqep(eqep_num);

  printf("SYSCONFIG 0x%X\n", *(uint32_t*)(eqep.getPWMSSPointer() + PWM_SYSCONFIG));
  printf("CLKCONFIG 0x%X\n", *(uint32_t*)(eqep.getPWMSSPointer()+PWM_CLKCONFIG));
  printf("QEPCTL0   0x%X\n",  eqep.getControl());
  printf("QDECCTL0  0x%X\n",  eqep.getDecoderControl());
  printf("QEINT0    0x%X\n",  eqep.getInterruptEnable());
  printf("QUPRD0    0x%u\n", eqep.getUnitPeriod());
  printf("QPOSMAX0  0x%X\n", eqep.getMaxPos());
  printf("QEPSTS0   0x%X\n",  eqep.getStatus());

  //time a read-loop to assess speed
  int num_reads = 1000000;
  int i;
  gettimeofday(&tv1,NULL);
  for(i=0;i<num_reads;i++){
    eqep_pos = eqep.getPosition();
  }
  gettimeofday(&tv2,NULL);

  //find difference between start and end time
  unsigned long dt_micros = (1000000 * tv2.tv_sec + tv2.tv_usec)-(1000000 * tv1.tv_sec + tv1.tv_usec);
  float time_per_read = (float)dt_micros/num_reads;
  float change_rate = (float)(num_reads)/((float)(dt_micros)) * 1000;

  printf("last position: %i\n", eqep_pos);
  printf("micros per read: %f\n", time_per_read);
  printf("quadrature read rate (kHz): %f\n",change_rate);
  printf("revid: 0x%X (should read 44D31103)\n",eqep.getRevisionID());

  eqep.setPosition(0);
  eqep_pos= eqep.getPosition();
  for(i=0;i<1000;i++){
	  gettimeofday(&tv1,NULL);
	  usleep(1000);
	  eqep_pos = eqep.getPosition();
//	  gettimeofday(&tv2,NULL);
//	  dt_micros = (1000000 * tv2.tv_sec + tv2.tv_usec)-(1000000 * tv1.tv_sec + tv1.tv_usec);
	  printf("\n%i:\t%i\t%0.2f\t%i", i, eqep_pos, eqep_pos * (360.0/1600.0), dt_micros);
  }
  printf("\n");

  return 0;
}

int main(int argc, char const *argv[]) {

	int motor_speed = 256;
	int target = 400;

	// Open serial port
	int fd = open_port();
	if (fd == -1) {
		perror("Couldn't open UART");
		return 1;
	}

	if (argc > 1) {
		motor_speed = strtol(argv[1],NULL,0);
		target = strtol(argv[2],NULL,0);
	}

	// Initialise eQEP
	BBB::eQEP eqep(MOTOR_EQEP);
	eqep.resetPositionCounter();			// reset eQEP
	eqep.positionCounterSourceSelection(0); // set Quadrature mode
	eqep.enablePositionCompareShadow();		// enable Shadow
	eqep.setCaptureLatchMode(BBB::eQEP::CLMCPU);
	eqep.enableCaptureUnit();
	eqep.setUnitPeriod(10000);

	// Initialise comms with motor controller
	smcAutoDetectBaudRate(fd);
	// Exit USB safe start
	smcExitSafeStart(fd);
	usleep(500);

	// Get voltage
	int vin = smcGetVariable(fd, 23);
	vin = smcGetVariable(fd, 23); // Read it twice just in case
	printf("Vin: %0.2f\n\n", vin/1000.0);


	smcSetTargetSpeed(fd, motor_speed);

	int pos = eqep.getPosition();
	int old_pos = 0;
	float angle = 0;
	int delta_pos = 0;
	int dir = 1;
	cout << "Position:" << pos << std::endl;
//	for (int i=0; i < 10; i++) {
		while (abs(pos) < abs(target)) {
			old_pos = pos;
			pos = eqep.getPosition();
			angle = pos/ENCODER_RATIO * 360;
			delta_pos = pos - old_pos;
			cout << "\r -- Position:" << pos << \
					"  Angle: " << angle << "deg " << \
					"  Capture Time: " << eqep.getCaptureTimerLatch() << \
					"  Capture Period: " << eqep.getCapturePeriodLatch() << \
					"  Delta  : " << delta_pos << "  ";
		}
//		target = -target;
//		motor_speed = -motor_speed;
//		smcSetTargetSpeed(fd, motor_speed);
//		while (pos > target) {
//			old_pos = pos;
//			pos = eqep.getPosition();
//			angle = pos/ENCODER_RATIO * 360;
//			delta_pos = pos - old_pos;
//			cout << "\r -- Position:" << pos << \
//					"  Angle: " << angle << "deg " << \
//					"  Capture Time: " << eqep.getCaptureTimerLatch() << \
//					"  Capture Period: " << eqep.getCapturePeriodLatch() << \
//					"  Delta  : " << delta_pos << "  ";
//		}
//		target = -target;
//		motor_speed = -motor_speed;
//		smcSetTargetSpeed(fd, motor_speed);
//	}
	smcSetTargetSpeed(fd, 0);
	cout << std::endl << "Position:" << eqep.getPosition() << endl;

	close(fd);
	return 0;
}