/*
 * pololu.cpp
 *
 *  Created on: 21 May 2015
 *      Author: troy
 */

#include <unistd.h>  // UNIX standard function definitions
#include <fcntl.h>   // file control definitions
#include <termios.h> // POSIX terminal control definitions
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>

// Language dependencies
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>  // Input-Output streams
#include <sstream>   // String stream
#include <fstream>   // File stream
#include <iomanip>	 // Stream IO manipulation
#include <math.h>

#include "pololu.h"
#include "bbb-eqep/bbb-eqep.h" // Memory mapped eQEP class
#include "BlackLib/BlackGPIO/BlackGPIO.h"
#include "BlackLib/BlackThread/BlackThread.h"
#include "pololu_serial.h"
#include "readEQEP.h"


using namespace std;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}


class blinkGPIO : public BlackLib::BlackThread {

private:
	BlackLib::BlackGPIO *GPIO;

public:

	blinkGPIO(BlackLib::gpioName __gpio) {
		GPIO = new BlackLib::BlackGPIO(__gpio, BlackLib::output);
	}

	void onStartHandler() {
		BlackLib::digitalValue value = BlackLib::low;
		for (int i=0; i < 10; i++) {
			if (value == BlackLib::low) {
				value = BlackLib::high;
			} else {
				value = BlackLib::low;
			}
			GPIO->setValue(value);
			sleep(1);
		}
	}

};

int main(int argc, char const *argv[]) {

	blinkGPIO *GPIO66 = new blinkGPIO(BlackLib::GPIO_66);
	readEQEP *MotorEQEP = new readEQEP(MOTOR_EQEP );

	GPIO66->run();
	MotorEQEP->run();

	BlackLib::BlackGPIO GPIO67(BlackLib::GPIO_67, BlackLib::output);

	GPIO67.setValue(BlackLib::high);
	sleep(2);
	cout << MotorEQEP->getPosition() << " " << MotorEQEP->getVelocity() << " " << MotorEQEP->getAngle()  << endl;
	GPIO67.setValue(BlackLib::low);
	sleep(2);
	cout << MotorEQEP->getPosition() << " " << MotorEQEP->getVelocity() << " " << MotorEQEP->getAngle()  << endl;
	GPIO67.setValue(BlackLib::high);
	sleep(2);
	cout << MotorEQEP->getPosition() << " " << MotorEQEP->getVelocity() << " " << MotorEQEP->getAngle()  << endl;
	GPIO67.setValue(BlackLib::low);

	if (MotorEQEP->isRunning()) {
		MotorEQEP->Exit();
	}

	WAIT_THREAD_FINISH(GPIO66);
	WAIT_THREAD_FINISH(MotorEQEP);
	return 0;
}
