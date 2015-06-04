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

#include <iostream>  // Input-Output streams

#include "BlackLib/BlackGPIO/BlackGPIO.h"		// GPIO access
#include "BlackLib/BlackThread/BlackThread.h"	// Thread class

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

	MotorEQEP->setPosition(0);

	for (int i=0; i < 10000; i++) {
		cout << "\r" << i << "  " << MotorEQEP->getAngleDeg() << "                ";
		usleep(50);
	}

	cout << endl << "Waiting for threads to finish ... ";
	if (MotorEQEP->isRunning()) {
		MotorEQEP->stop();
	}

	WAIT_THREAD_FINISH(GPIO66);
	WAIT_THREAD_FINISH(GPIO67);
	WAIT_THREAD_FINISH(MotorEQEP);
	cout << "Done!" << endl;
	return 0;
}
