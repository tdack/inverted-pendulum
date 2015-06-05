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
#include <cmath>
#include "pololu.h"
#include "threadedEQEP.h"
#include "BlackLib/BlackGPIO/BlackGPIO.h"		// GPIO access
#include "pid.h"

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

	threadedEQEP *pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, ENCODER_PPR);

	pendulumEQEP->run();

	cout << "Raise pendulum" << endl;

	while (abs(pendulumEQEP->getAngleDeg()) < 178 || abs(pendulumEQEP->getAngleDeg() > 182))  {
		cout << "\r    " << pendulumEQEP->getAngleDeg();
		usleep(20);
	}

	cout << "Vertical! \n Starting!" << endl;
	pendulumEQEP->stop();

	pid *Controller = new pid(11.7, 50, 8, 40);

	Controller->run();

	sleep(30);

	Controller->stop();
	WAIT_THREAD_FINISH(Controller);

	cout << "Done!" << endl;
	return 0;
}
