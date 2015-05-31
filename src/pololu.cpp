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
//#include "BlackLib/BlackCore.h"
//#include "BlackLib/BlackPWM.h"
//#include "BlackLib/BlackGPIO.h"

#include "pololu_serial.h"

using std::cout;
using std::endl;
using std::cerr;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

// Need global variable so PWM overlay doesn't keep being loaded and unloaded
//BlackLib::BlackPWM pwm(BlackLib::EHRPWM2A); // P8_19

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

//void beep(int count=1){
//
//	pwm.setDutyPercent(15.0);
//	pwm.setPeriodTime(10000);
//	for (int i=0; i < count; i++) {
//		pwm.setRunState(BlackLib::run);
//		usleep(75000);
//		pwm.setRunState(BlackLib::stop);
//		usleep(75000);
//	}
//	pwm.setDutyPercent(0.0);
//
//}

int oscillate_motor(int argc, char const *argv[]) {
	int motor_speed = 256;
	int target = 90; // target position in deg

	// Open serial port
	int fd = open_port();
	if (fd == -1) {
		perror("Couldn't open UART");
		return 1;
	}

	motor_speed = strtol(argv[1],NULL,0);
	target = strtol(argv[2],NULL,0);
	if (target < 0 || target > 360) {
		target = 90;
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
	cout << "Current errors " << smcGetVariable(fd, 1) << endl;
	usleep(500);

	int pos = eqep.getPosition();
	int old_pos = 0;
	int delta_pos = 0;
	int error_status = 0;
	float angle = 0.0;
	float old_angle = 0.0;
	float delta_angle = 0.0;

	for (int i=0; i<10; i++) {
		smcSetTargetSpeed(fd, motor_speed);
		error_status = smcGetErrorStatus(fd);
		cout << "\nError status " << std::cout.write(reinterpret_cast<const char*>(&error_status), sizeof error_status) << endl;
		while (angle < target) {
			old_pos = pos;
			old_angle = angle;
			pos = eqep.getPosition();
			angle = pos/ENCODER_RATIO * 360;
			delta_pos = pos - old_pos;
			delta_angle = angle - old_angle;
			cout << "\r -- Position: " << std::setw(5) << std::setfill(' ') << pos << \
					"  Angle: " << std::fixed << std::setw(8) << std::setprecision(2) << std::setfill(' ') << angle << " deg " << \
					"  Delta  : " << std::fixed << std::setw(8) << std::setprecision(2) << std::setfill(' ') << delta_angle << " deg";
			if ((target - angle) < (target*0.1)) {
				smcSetTargetSpeed(fd, motor_speed < 512 ? sgn(motor_speed) * 128 : motor_speed/4);
			}
		}
		motor_speed = -motor_speed;
		target = -target;
		smcSetTargetSpeed(fd, motor_speed);
		error_status = smcGetErrorStatus(fd);
		cout << "\nError status " << std::cout.write(reinterpret_cast<const char*>(&error_status), sizeof error_status) << endl;
		while (angle > target) {
			old_pos = pos;
			old_angle = angle;
			pos = eqep.getPosition();
			angle = pos/ENCODER_RATIO * 360;
			delta_pos = pos - old_pos;
			delta_angle = angle - old_angle;
			cout << "\r -- Position: " << std::setw(5) << std::setfill(' ') << pos << \
					"  Angle: " << std::fixed << std::setw(8) << std::setprecision(2) << std::setfill(' ') << angle << " deg " << \
					"  Delta  : " << std::fixed << std::setw(8) << std::setprecision(2) << std::setfill(' ') << delta_angle << " deg";
			if ((target - angle) > (target*0.1)) {
				smcSetTargetSpeed(fd, motor_speed < 512 ? sgn(motor_speed) * 128 : motor_speed/4);
			}
		}
		motor_speed = -motor_speed;
		target = -target;
	}
	smcSetTargetSpeed(fd, 0);
	cout << "\n" << eqep.getPosition() << endl;

	close(fd);
	return 0;
}

int step_motor(int argc, char const *argv[]) {
	int motor_speed = 256;
	int duration = 5; // Duration of step
	struct timeval tv1, tv2;

	// Open serial port
	int fd = open_port();
	if (fd == -1) {
		perror("Couldn't open UART");
		return 1;
	}

	motor_speed = strtol(argv[1],NULL,0);
	duration = strtol(argv[2],NULL,0);
	if (duration < 0 || duration > 60) {
		duration = 5;
	}

	motor_speed = abs(motor_speed) > 3200 ? 3200 : motor_speed;
	motor_speed = abs(motor_speed) < 128 ? 128 : motor_speed;

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

	int pos = eqep.getPosition();
	int old_pos = 0;
	int delta_pos = 0;
	int error_status = 0;
	float angle = 0.0;
	float old_angle = 0.0;
	float delta_angle = 0.0;

	smcSetTargetSpeed(fd, motor_speed);
	gettimeofday(&tv1,NULL);
	unsigned long dt_micros=0;
	// Turn off output buffering (hopefully)
	std::cout.setf(std::ios::unitbuf);
	cout << "Position,Angle,Microseconds" << endl;
	while ( dt_micros < (duration*1000000)) {
		old_pos = pos;
		old_angle = angle;
		pos = eqep.getPosition();
		gettimeofday(&tv2,NULL);
		dt_micros = (1000000 * tv2.tv_sec + tv2.tv_usec)-(1000000 * tv1.tv_sec + tv1.tv_usec);
		angle = pos/ENCODER_RATIO * 360;
		delta_pos = pos - old_pos;
		delta_angle = angle - old_angle;
		cout << pos << "," << \
			angle << "," << \
			dt_micros << endl;
		// sleep for 1ms to allow for a reasonable position change between each read
		usleep(1000);
	}
	smcSetTargetSpeed(fd, 0);

	close(fd);
	return 0;
}

int main(int argc, char const *argv[]) {

	if (argc == 4) {
		if (strtol(argv[3],NULL,0) == 1) {
			return step_motor(argc, argv);
		} else if (strtol(argv[3],NULL,0) == 2) {
			return oscillate_motor(argc, argv);
		}
	} else {
		cout << "Usage" << endl;
		cout << "      Step motor     : " << argv[0] << " [speed] [duration] 1" << endl;
		cout << "      Osscilate motor: " << argv[0] << " [speed] [angle] 2" << endl;
	}
	return 0;
}
