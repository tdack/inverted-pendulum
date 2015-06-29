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

#include <BlackLib/BlackThread/BlackThread.h>
#include <pid.h>
#include <pololu.h>
#include <rlutil.h>
#include <threadedEQEP.h>
#include <unistd.h>
#include <cmath>
#include <iostream>  // Input-Output streams
#include <sys/stat.h>

using namespace std;


void controller() {

	// Create new EQEPs object to monitor the pendulum position
	threadedEQEP *pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, ENCODER_PPR);

	// Start the thread running
	pendulumEQEP->run();
	rlutil::cls();
	rlutil::setColor(rlutil::BLUE);
	cout << "Raise the pendulum" << endl;
	rlutil::setColor(rlutil::WHITE);

	// Wait until the pendulum is @ 180 +-1 deg
	// Assumes pendulum starts hanging vertically down
	while (abs(pendulumEQEP->getAngleDeg()) < 179 || abs(pendulumEQEP->getAngleDeg() > 181))  {
		cout << "\r    " << pendulumEQEP->getAngleDeg();
		usleep(20);
	}
	pendulumEQEP->stop();

	rlutil::cls();
	rlutil::setColor(rlutil::GREEN);
	cout << "\n\n Vertical! .. \n Starting!" << endl;
	rlutil::setColor(rlutil::WHITE);

	// Create a new PID controller thread
	pid *Controller = new pid(11.7, 50, 8, 40);

	Controller->run();

	// Let the thread run for 30 seconds
	sleep(30);

	Controller->stop();

	// Don't quit until all threads are finished
	WAIT_THREAD_FINISH(Controller);
	WAIT_THREAD_FINISH(pendulumEQEP);

	cout << "Done!" << endl;
	return;
}

bool checkOverlays(){
	std::vector<std::string> files = {
			POLOLU_TTY,								   // tty device path
			"/sys/bus/platform/devices/48300180.eqep", // eqep device path
			"/sys/bus/platform/devices/48302180.eqep"  // eqep device path
	};

	struct stat buffer;
	for (std::string& file : files) {
		if (stat(file.c_str(), &buffer) != 0) {
			cout << file << " not found.  Are the overlays loaded?" << std::endl;
			return false;
		}
	}
	return true;
}

int main(int argc, char const *argv[]) {

	if (!checkOverlays()) {
		return -1;
	}
	// Create a Simple Motor Controller object
	Pololu::SMC *SMC = new Pololu::SMC(POLOLU_TTY);

	SMC->SetTargetSpeed(0);

	usleep(500);
	SMC->SetTargetSpeed(256);

	sleep(3);
	SMC->SetTargetSpeed(-256);

	sleep(3);
	SMC->SetTargetSpeed(0);

	controller();

	return 0;
}
