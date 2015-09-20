/**
 * @file
 * @brief Inverted Pendulum main file
 *
 * @author Troy Dack
 * @date Copyright (C) 2015
 *
 * @license
 * \verbinclude "Troy Dack GPL-2.0.txt"
 *
 **/

#include <pendulum.h>
#include <overlays.h>

#include <Controller/basic.h>
#include <Controller/velocity.h>
#include <Controller/lqr.h>

using namespace std;

void controller(double kp, double ki, double kd, int dir) {

	// Initialise display
	SSD1306::OLED *outLED = new SSD1306::OLED();

	// Display initial message
	outLED->fx->clearScreen();
	outLED->fx->setTextColor(SSD1306::RGB::black, SSD1306::RGB::white);
	outLED->fx->setTextSize(1);
	outLED->fx->setCursor(2,4);
	outLED->fx->write(" Raise the pendulum");
	outLED->fx->setCursor(0, 24);
	outLED->fx->write(" P:\n M:\n S:");
	outLED->fx->drawRoundRect(0,0,outLED->fx->getWidth(), 16, 4, SSD1306::RGB::black);
	std::cout << "Raise the pendulum" << std::endl;

	// Variables that will be used to pass data to/from controller
	double pendulumAngle = 0;
	double pendulumVelocity =0;
	double motorAngle = 0;
	double motorVelocity =0;
	double motorSpeed = 0;
	double setAngle = 0;
	int count = 0;
	int setSpeed;

	// Measure time in processing loop
	std::chrono::high_resolution_clock::time_point lastTime;
	std::chrono::duration<float, std::deci> timeChange;
	std::chrono::duration<float, std::deci> runTime;
	std::chrono::high_resolution_clock::time_point now;
	std::chrono::high_resolution_clock::time_point start;
	now = std::chrono::high_resolution_clock::now();
	timeChange = (now - lastTime);

	// Create new EQEPs object to monitor the pendulum & motor position
	threadedEQEP *pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, ENCODER_PPR);
	threadedEQEP *motorEQEP = new threadedEQEP(MOTOR_EQEP, MOTOR_PPR);

	// Create a new controller
//	Controller::basic *ctrl = new Controller::basic(&pendulumAngle, &motorSpeed, &setAngle, kp, ki, kd, dir);
	Controller::velocity *ctrl = new Controller::velocity(&pendulumAngle,&pendulumVelocity, &motorSpeed, &setAngle, kp, ki, kd, dir);
//	Controller::lqr *ctrl = new Controller::lqr(&pendulumAngle, &pendulumVelocity,
//										&motorAngle, &motorVelocity,
//										&motorSpeed, &setAngle,
//										-23.1455, 126.3112, -5.7435, 7.5213, // LQR constants
//										dir);

	// Create a Simple Motor Controller object
	Pololu::SMC *SMC = new Pololu::SMC(POLOLU_TTY);
	// Stop the motor
	SMC->SetTargetSpeed(0);

	// Start the EQEP threads running
	pendulumEQEP->run();
	motorEQEP->run();

	// Wait until the pendulum is @ 180 +-1 deg
	// Assumes pendulum starts hanging vertically down
	while (abs(pendulumEQEP->getAngleDeg()) < 179 || abs(pendulumEQEP->getAngleDeg() > 181))  {
		outLED->fx->setCursor(18, 24);
		outLED->fx->write(to_string(pendulumEQEP->getAngleDeg()).c_str());
		outLED->fx->refreshScreen();
	}

	// Set controller parameters
	ctrl->SetOutputLimits(-3200.0,3200.0);
	ctrl->SetSampleTime(15); // sample time in milliseconds
	ctrl->SetMode(1); // Automatic


	// Reset pendulum position to make vertical zero
	pendulumEQEP->setPosition(180-abs(pendulumEQEP->getAngleDeg()));
	motorEQEP->setPosition(0);

	// start the controller thread
	ctrl->run();

	outLED->fx->setCursor(2,4);
	outLED->fx->write("Controller Running ");
	std::cout << "Controller Running ...." << std::endl;
	start = lastTime = std::chrono::high_resolution_clock::now();
	// Let the threads run for about 90 seconds
	do {
		now = std::chrono::high_resolution_clock::now();
		timeChange = (now - lastTime);
		// Get pendulum angle and velocity
		pendulumAngle = pendulumEQEP->getAngle();
		pendulumVelocity = pendulumEQEP->getVelocity();
		// Get motor angle and velocity
		motorAngle = motorEQEP->getAngleDeg();
		motorVelocity = motorEQEP->getVelocityDeg();

		// Motor doesn't move unless speed > 300
		setSpeed = ( motorSpeed > 0 ? 1 : -1) * 300 + (int)motorSpeed;

		// stop the motor if we deviate too far from vertical
		if (abs(pendulumAngle * 180 / M_PI) > 25) { // convert radian to degrees
//		if (abs(pendulumAngle) > 25) {
			SMC->SetTargetSpeed(0);
		} else {
			SMC->SetTargetSpeed(setSpeed);
		}

		// Use threaded SSD1306 so that screen updates don't slow down control loop
		outLED->write(18,24, to_string(pendulumEQEP->getAngleDeg()).c_str());
		outLED->write(18,32, to_string(motorEQEP->getAngleDeg()).c_str());
		outLED->write(18,40, to_string(setSpeed).c_str());
		outLED->write(-1,-1, "   ");
		outLED->write(110,54, to_string(count).c_str());
		outLED->write(-1, -1, "  ");
		outLED->run();
		lastTime = now;
		runTime = (now - start);
	} while (runTime.count() < 90);

	SMC->SetTargetSpeed(0);

	outLED->stop();
	ctrl->stop();
	pendulumEQEP->stop();
	motorEQEP->stop();

	// Don't quit until all threads are finished
	WAIT_THREAD_FINISH(outLED);
	WAIT_THREAD_FINISH(ctrl);
	WAIT_THREAD_FINISH(pendulumEQEP);
	WAIT_THREAD_FINISH(motorEQEP);

	delete ctrl;
	delete pendulumEQEP;
	delete motorEQEP;

	cout << "Done!" << endl;
	return;
}

int main(int argc, char const *argv[]) {
	std::vector<std::string> args(argv +1, argv + argc);

	std::cout << "Checking overlays are loaded... \n" << std::flush;

	if (checkOverlays()) {
		cout << "OK" << std::endl;
	} else {
		return 0;
	}

	if (args.size() == 4) {
		controller(atof(args[0].c_str()), atof(args[1].c_str()), atof(args[2].c_str()), atoi(args[3].c_str()));
	} else {
		controller(1,0,0,0);
	}

	return 0;
}
