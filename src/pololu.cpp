/**
 *! @file pololu.cpp
 *! Inverted Pendulum
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

#include <unistd.h>  // UNIX standard function definitions
#include <fcntl.h>   // file control definitions
#include <iostream>  // Input-Output streams
#include <termios.h> // POSIX terminal control definitions
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>

// Language dependencies
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <math.h>

#include "pololu.h"
#include "bbb-eqep/bbb-eqep.h"
#include "bbb-eqep/debug.h"
//#include "BlackLib/BlackCore.h"
//#include "BlackLib/BlackPWM.h"
//#include "BlackLib/BlackGPIO.h"

#include "pololu_serial.h"

using namespace std;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

struct thread_data {
	int speed;
	int target;
};
struct thread_data oscillate_thread_data;

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

void *oscillate_motor(void *threadarg) {

	struct thread_data *my_data;
	my_data = (struct thread_data *) threadarg;
	pthread_t tid;
	tid = pthread_self();

	cout << "tid: " << tid << endl;

	int motor_speed;
	int target; // target position in deg

	motor_speed = my_data->speed;
	target = my_data->target;
	if (target < -360 || target > 360) {
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

	// Initialise motor controller
	Pololu::SMC SMC(POLOLU_TTY);
	cout << "Current errors " << SMC.GetVariable(1) << endl;
	usleep(500);

	int pos = eqep.getPosition();
	int old_pos = 0;
	int delta_pos = 0;
	int error_status = 0;
	float angle = 0.0;
	float old_angle = 0.0;
	float delta_angle = 0.0;

	for (int i=0; i<10; i++) {
		SMC.SetTargetSpeed(motor_speed);
		error_status = SMC.GetErrorStatus();
		cout << "\nError status " << cout.write(reinterpret_cast<const char*>(&error_status), sizeof error_status) << endl;
		while (angle < target) {
			old_pos = pos;
			old_angle = angle;
			pos = eqep.getPosition();
			angle = pos/ENCODER_RATIO * 360;
			delta_pos = pos - old_pos;
			delta_angle = angle - old_angle;
			cout << "\r -- Position: " << setw(5) << setfill(' ') << pos << \
					"  Angle: " << fixed << setw(8) << setprecision(2) << setfill(' ') << angle << " deg " << \
					"  Delta  : " << fixed << setw(8) << setprecision(2) << setfill(' ') << delta_angle << " deg";
			if ((target - angle) < (target*0.1)) {
				SMC.SetTargetSpeed(motor_speed < 512 ? sgn(motor_speed) * 128 : motor_speed/4);
			}
		}
		motor_speed = -motor_speed;
		target = -target;
		SMC.SetTargetSpeed(motor_speed);
		error_status = SMC.GetErrorStatus();
		cout << "\nError status " << cout.write(reinterpret_cast<const char*>(&error_status), sizeof error_status) << endl;
		while (angle > target) {
			old_pos = pos;
			old_angle = angle;
			pos = eqep.getPosition();
			angle = pos/ENCODER_RATIO * 360;
			delta_pos = pos - old_pos;
			delta_angle = angle - old_angle;
			cout << "\r -- Position: " << setw(5) << setfill(' ') << pos << \
					"  Angle: " << fixed << setw(8) << setprecision(2) << setfill(' ') << angle << " deg " << \
					"  Delta  : " << fixed << setw(8) << setprecision(2) << setfill(' ') << delta_angle << " deg";
			if ((target - angle) > (target*0.1)) {
				SMC.SetTargetSpeed(motor_speed < 512 ? sgn(motor_speed) * 128 : motor_speed/4);
			}
		}
		motor_speed = -motor_speed;
		target = -target;
	}
	SMC.SetTargetSpeed(0);
	cout << "\n" << eqep.getPosition() << endl;
	pthread_exit(NULL);
}

/**
 * Apply a step voltage to the motor for a period of time.
 *
 * Essentially motor is energised at maximum by motor controller.
 *
 * Output data is stored in CSV format for analysis using MATLAB or Excel
 * @param argc
 * @param argv
 * @return
 */
int step_motor(int argc, char const *argv[]) {
	int motor_speed;
	int duration; // Duration of step
	struct timeval tv1, tv2;

	motor_speed = strtol(argv[2],NULL,0);
	duration = strtol(argv[3],NULL,0);
	if (duration < 0 || duration > 30) {
		duration = 5;
	}

	// Create output file
	ofstream results;
	if (strlen(argv[4]) != 0) {
		results.open(argv[4], ios::out | ios::trunc);
	} else {
		cerr << "No filename given for results" << endl;
		return -1;
	}
	// Make sure motor speed is in valid range, otherwise coerce it.
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

	// Initialise motor controller
	Pololu::SMC SMC(POLOLU_TTY);
	// Wait for a bit before starting
	usleep(500);

	int pos = eqep.getPosition();
	int old_pos = 0;
	int dt_pos = 0;
	float angle = 0.0;
	float old_angle = 0.0;
	float dt_angle = 0.0;
	float velocity = 0;

	SMC.SetTargetSpeed(motor_speed);
	gettimeofday(&tv1,NULL);
	unsigned long dt_micros = 0;
	unsigned long old_dt_micros = 0;
	float dt = 0.0;
	results << "dt_pos,dt_angle (rad),dt (us),t (s),v (rad/s)" << endl;
	while ( dt_micros < (duration*1000000)) {
		old_pos = pos;
		old_angle = angle;
		old_dt_micros = dt_micros;
		pos = eqep.getPosition();
		gettimeofday(&tv2,NULL);
		dt_micros = (1000000 * tv2.tv_sec + tv2.tv_usec)-(1000000 * tv1.tv_sec + tv1.tv_usec);
		dt = (dt_micros - old_dt_micros)/1e6; // get delta time in seconds
		angle = pos/ENCODER_RATIO * 2 * M_PI; // angle in radians
		dt_pos = pos - old_pos;
		dt_angle = angle - old_angle;
		velocity = dt_angle/dt; // velocity in radians/second
		results << dt_pos << ", " << dt_angle << ", " << dt << ", " << dt_micros/1e6 <<", " << velocity << endl;
		// sleep for ~5ms to allow for a reasonable position change between each read
		// this ensures that we actually get a velocity
		usleep(5000);
	}
	SMC.SetTargetSpeed(0);
	results.close();

	return 0;
}

int main(int argc, char const *argv[]) {

	pthread_t oscillate_thread;
	int rc;

	if (argc > 2) {
		if (strtol(argv[1],NULL,0) == 1) {
			return step_motor(argc, argv);
		} else if (strtol(argv[1],NULL,0) == 2) {
			oscillate_thread_data.speed = strtol(argv[2],NULL,0);
			oscillate_thread_data.target = strtol(argv[3],NULL,0);
			rc = pthread_create(&oscillate_thread, NULL, oscillate_motor, (void *) &oscillate_thread_data);
			if (rc) {
				cout << "Thread creation failed!!" << endl;
				return 0;
			}
			for (int i=0; i<100; i++) {
//				usleep(1000);
				if (i % 10) {
					cout << "...";
				}
			}
			pthread_join(oscillate_thread, NULL);
		}
	} else {
		cout << "Usage" << endl;
		cout << "      Step motor     : " << argv[0] << " 1 [speed] [duration] [filename]" << endl;
		cout << "      Osscilate motor: " << argv[0] << " 2 [speed] [angle]" << endl;
	}
	return 0;
}
