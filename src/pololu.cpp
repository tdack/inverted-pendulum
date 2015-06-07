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

int main(int argc, char const *argv[]) {

	// Create a new EQEP object to monitor the pendulum position
	threadedEQEP *pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, ENCODER_PPR);

	// Start the thread running
	pendulumEQEP->run();

	cout << "Raise the pendulum" << endl;

	// Wait until the pendulum is @ 180 +-2 deg
	// Assumes pendulum starts hanging vertically down
	while (abs(pendulumEQEP->getAngleDeg()) < 178 || abs(pendulumEQEP->getAngleDeg() > 182))  {
		cout << "\r    " << pendulumEQEP->getAngleDeg();
		usleep(20);
	}

	cout << "Vertical! \n Starting!" << endl;
	pendulumEQEP->stop();

	// Create a new PID controller thread
	pid *Controller = new pid(11.7, 50, 8, 40);

	Controller->run();

	// Let the thread run for 30 seconds
	sleep(30);

	Controller->stop();
	WAIT_THREAD_FINISH(Controller);

	cout << "Done!" << endl;
	return 0;
}
