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
#include <iomanip>

#include "pololu.h"
#include "bbb-eqep/bbb-eqep.h"
#include "bbb-eqep/debug.h"
#include "BlackLib/BlackCore.h"
#include "BlackLib/BlackPWM.h"
#include "BlackLib/BlackGPIO.h"

#include "pololu_serial.h"

using std::cout;
using std::endl;
using std::cerr;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

int open_port(){
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

void beep(int count=1){

	BlackLib::BlackPWM pwm(BlackLib::EHRPWM2A); // P8_19

	pwm.setDutyPercent(15.0);
	pwm.setPeriodTime(10000);
	for (int i=0; i < count; i++) {
		pwm.setRunState(BlackLib::run);
		usleep(75000);
		pwm.setRunState(BlackLib::stop);
		usleep(75000);
	}
	pwm.setDutyPercent(0.0);

}

int main(int argc, char const *argv[]) {

	beep(3);
	int motor_speed = 256;
	int target = -90; // target position in deg

	// Open serial port
	int fd = open_port();
	if (fd == -1) {
		perror("Couldn't open UART");
		return 1;
	}

	//
	if (argc == 3) {
		motor_speed = strtol(argv[1],NULL,0);
		target = strtol(argv[2],NULL,0);
		if (target < 0 || target > 360) {
			target = 90;
		}
	} else if (argc == 2) {
		motor_speed = strtol(argv[1],NULL,0);
	}

	motor_speed = abs(motor_speed) > 3200 ? sgn(motor_speed) * 256 : motor_speed;
	motor_speed = abs(motor_speed) < 128 ? sgn(motor_speed) * 128 : motor_speed;

	// Initialise eQEP
	BBB::eQEP eqep(MOTOR_EQEP);
	eqep.resetPositionCounter();			// reset eQEP
	eqep.positionCounterSourceSelection(0); // set Quadrature mode
	eqep.enablePositionCompareShadow();		// enable Shadow
	eqep.setCaptureLatchMode(BBB::eQEP::CLMCPU);
	eqep.enableCaptureUnit();
	eqep.setUnitPeriod(1000);

	// Initialise comms with motor controller
	smcAutoDetectBaudRate(fd);
	// Exit USB safe start
	smcExitSafeStart(fd);
	// Wait for a bit before starting
	usleep(500);

	smcSetTargetSpeed(fd, motor_speed);
	int pos = eqep.getPosition();
	int old_pos = 0;
	float angle = 0;
	int delta_pos = 0;

	while (abs(angle) < abs(target)) {
		old_pos = pos;
		pos = eqep.getPosition();
		angle = pos/ENCODER_RATIO * 360;
		delta_pos = pos - old_pos;
		cout << "\r -- Position: " << std::setw(5) << std::setfill(' ') << pos << \
				"  Angle: " << std::fixed << std::setw(8) << std::setprecision(2) << std::setfill(' ') << angle << " deg " << \
				"  Delta  : " << delta_pos;
		if ((abs(target)-abs(angle)) < (abs(target)*0.1)) {
			smcSetTargetSpeed(fd, motor_speed < 512 ? sgn(motor_speed) * 128 : motor_speed/4);
		}
	}
	smcSetTargetSpeed(fd, 0);
	beep();
	cout << endl;

	close(fd);
	return 0;
}
