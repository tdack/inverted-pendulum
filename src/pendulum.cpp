/**
 * @file
 * @brief Inverted Pendulum main file
 *
 * @author Troy Dack
 * @date Copyright (C) 2015
 *
 * @license
 * \verbinclude "Troy Dack - GPL-2.0.txt"
 *
 **/

#define DEBUG

#include <pendulum.h>
#include <overlays.h>
#include <thread>

#include <Controller/basic.h>
#include <Controller/velocity.h>
#include <Controller/lqr.h>

using namespace std;

/*! @brief Set frequency and output duration for PWM
*
* @param f Frequency in Hz
* @param d Duration in milliseconds
*/
void buzzer(int f, int d) {
	BlackLib::BlackPWM buzzerPWM(BlackLib::P8_19);

	buzzerPWM.setDutyPercent(50.0);
	buzzerPWM.setPeriodTime(1/f, BlackLib::second);
	buzzerPWM.setRunState(BlackLib::run);
	std::this_thread::sleep_for(std::chrono::milliseconds(d));
	buzzerPWM.setRunState(BlackLib::stop);
	return;
}

/*!
 * @brief Main controller loop
 *  Initialises display, eQEPs and controller.  Waits for user to raise pendulum
 *  			then runs main control loop and sets motor speed based on controller output
 *
 * @param kp Proportional constant for PID controller
 * @param ki Integral constant for PID controller
 * @param kd Derivative constant for PID controller
 * @param dir Direction (0 or 1) that controller should operate in.
 */
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
	double pendulumAngleDeg = 0;
	double pendulumVelocity =0;
	double motorAngle = 0;
	double motorVelocity =0;
	double motorSpeed = 0;
	double setAngle = 0;
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
	Controller::basic *ctrl = new Controller::basic(&pendulumAngle, &motorSpeed, &setAngle, kp, ki, kd, dir);
//	Controller::velocity *ctrl = new Controller::velocity(&pendulumAngle,&pendulumVelocity, &motorSpeed, &setAngle, kp, ki, kd, dir);
//	Controller::lqr *ctrl = new Controller::lqr(&pendulumAngle, &pendulumVelocity,
//										&motorAngle, &motorVelocity,
//										&motorSpeed, &setAngle,
//										-23.1455, 126.3112, -5.7435, 7.5213, // LQR constants
//										dir);

	std::cout << "Using " << "Basic" << " controller" << std::endl;

	// Create a Simple Motor Controller object
	Pololu::SMC *SMC = new Pololu::SMC(POLOLU_TTY);
	// Stop the motor
	SMC->SetTargetSpeed(0);
	std::cout << "Battery Voltage: " << SMC->GetVariable(23)/1000.0 << "V" << std::endl;
	cout << "Done!" << endl;

	// Start the EQEP threads running
	pendulumEQEP->run();
	motorEQEP->run();
	buzzer(3500, 25);

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
	buzzer(3500, 15);
	outLED->fx->setCursor(2,4);
	outLED->fx->write("Controller Running ");
	std::cout << "Controller Running ...." << std::endl;
	buzzer(3500, 15);

	start = lastTime = std::chrono::high_resolution_clock::now();
	// Let the threads run for about 90 seconds
	do {
		now = std::chrono::high_resolution_clock::now();
		timeChange = (now - lastTime);
		// Get pendulum angle and velocity
		pendulumAngle = pendulumEQEP->getAngle();
		pendulumAngleDeg = pendulumAngle * 180 / M_PI; // convert radian to degrees
		pendulumVelocity = pendulumEQEP->getVelocity();
		// Get motor angle and velocity
		motorAngle = motorEQEP->getAngleDeg();
		motorVelocity = motorEQEP->getVelocityDeg();

		// Motor doesn't move unless speed > 350
		setSpeed = ( motorSpeed > 0 ? 1 : -1) * 350 + (int)motorSpeed;

		// stop the motor if we deviate too far from vertical
		SMC->SetTargetSpeed( ((abs(pendulumAngleDeg) > 30) ? 0 : setSpeed) );

		// Use threaded SSD1306 so that screen updates don't slow down control loop
		outLED->write(18,24, to_string(pendulumAngleDeg).c_str());
		outLED->write(18,32, to_string(motorAngle).c_str());
		outLED->write(18,40, to_string(setSpeed).c_str());
		outLED->write(-1,-1, "   ");
		outLED->run();
		lastTime = now;
		runTime = (now - start);
	} while (runTime.count() < 90);
	buzzer(5000, 25);

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

	SMC->SetTargetSpeed(0);
	std::cout << "Battery Voltage: " << SMC->GetVariable(23)/1000.0 << "V" << std::endl;
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
