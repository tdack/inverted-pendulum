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

#include <BlackLib/BlackDef.h>
#include <BlackLib/BlackGPIO/BlackGPIO.h>
#include <BlackLib/BlackThread/BlackThread.h>
#include <pid.h>
#include <pololu.h>
#include <pololu_serial.h>
#include <rlutil.h>
#include <sys/stat.h>
#include <threadedEQEP.h>
#include <unistd.h>
#include <cmath>
#include <cstdbool>
#include <iostream>
#include <string>
#include <vector>

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
	bool overlays_loaded = true;

	rlutil::cls();
	rlutil::setColor(rlutil::RED);
	for (std::string& file : files) {
		if (stat(file.c_str(), &buffer) != 0) {
			rlutil::setColor(rlutil::YELLOW);
			cout << file << " ";
			rlutil::setColor(rlutil::RED);
			cout << "not found." << endl;;
			overlays_loaded = false;
		}
	}
	return overlays_loaded;
}


int main(int argc, char const *argv[]) {

	if (!checkOverlays()) {
		rlutil::setColor(rlutil::WHITE);
		cout << "Are the overlays loaded?" << std::endl;
		return -1;
	}

	BlackLib::BlackGPIO P8_7(BlackLib::GPIO_66, BlackLib::output);
	BlackLib::BlackGPIO P8_8(BlackLib::GPIO_67, BlackLib::output);

	P8_7 << BlackLib::high;
	P8_8 << BlackLib::high;

	// Create a Simple Motor Controller object
	Pololu::SMC *SMC = new Pololu::SMC(POLOLU_TTY);

	SMC->SetTargetSpeed(0);

	usleep(500);
	SMC->SetTargetSpeed(256);
	P8_7 << BlackLib::low;
	P8_8 << BlackLib::high;

	sleep(3);
	SMC->SetTargetSpeed(-256);
	P8_7 << BlackLib::high;
	P8_8 << BlackLib::low;

	sleep(3);
	SMC->SetTargetSpeed(0);

	P8_7 << BlackLib::low;
	P8_8 << BlackLib::low;

	controller();

	return 0;
}
