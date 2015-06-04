/*
 * pololu.cpp
 *
 *  Created on: 21 May 2015
 *      Author: troy
 */

#include <iostream>  // Input-Output streams

#include "BlackLib/BlackGPIO/BlackGPIO.h"		// GPIO access
//#include "BlackLib/BlackThread/BlackThread.h"	// Thread class

#include "pololu.h"
#include "threadedEQEP.h"

using namespace std;

class blinkGPIO : public BlackLib::BlackThread {

private:
	BlackLib::BlackGPIO *GPIO;
	int delay;
public:

	blinkGPIO(BlackLib::gpioName __gpio, int delay) {
		GPIO = new BlackLib::BlackGPIO(__gpio, BlackLib::output);
		this->delay = delay;
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
			sleep(delay);
		}
	}

};

int main(int argc, char const *argv[]) {

	blinkGPIO *GPIO66 = new blinkGPIO(BlackLib::GPIO_66, 1);
	blinkGPIO *GPIO67 = new blinkGPIO(BlackLib::GPIO_67, 2);

	threadedEQEP *MotorEQEP = new threadedEQEP(MOTOR_EQEP, ENCODER_PPR);

	GPIO66->run();
	GPIO67->run();
	MotorEQEP->run();

	cout << endl;

	while (MotorEQEP->getAngleDeg() < 178 || MotorEQEP->getAngleDeg() > 182)  {
		cout << "\r" << MotorEQEP->getAngleDeg();
		usleep(20);
	}

	cout << "Vertical! \n Starting!" << endl;

	for (int i=0; i < 10000; i++) {
		cout << "\r" << i << "  " << MotorEQEP->getAngleDeg() << "                ";
		usleep(50);
	}

	if (MotorEQEP->isRunning()) {
		MotorEQEP->stop();
	}

	WAIT_THREAD_FINISH(GPIO66);
	WAIT_THREAD_FINISH(GPIO67);
	WAIT_THREAD_FINISH(MotorEQEP);
	return 0;
}
