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

// Conditional defines determine which controller will be used
#ifdef PENDULUM_CTRL_LQR
#include <Controller/lqr.h>

#elif PENDULUM_CTRL_VELOCITY
#include <Controller/velocity.h>

#else //Use basic controller by default
#define PENDULUM_CTRL_BASIC
#include <Controller/basic.h>

#endif

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

	std::cout << "Raise the pendulum" << std::endl;

	// Variables that will be used to pass data to/from controller
	double pendulumAngle = 0;
	double pendulumAngleDeg = 0;
	double pendulumVelocity =0;
	double motorAngle = 0;
	double motorAngleDeg = 0;
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

	pendulumEQEP->setPriority(BlackLib::BlackThread::PriorityNORMAL);
	motorEQEP->setPriority(BlackLib::BlackThread::PriorityNORMAL);

	// Create a new controller
#ifdef PENDULUM_CTRL_LQR
	Controller::lqr *ctrl = new Controller::lqr(&pendulumAngle, &pendulumVelocity,
										&motorAngle, &motorVelocity,
										&motorSpeed, &setAngle,
										-23.1455, 126.3112, -5.7435, 7.5213, // LQR constants
										dir);
#elif PENDULUM_CTRL_VELOCITY
	Controller::velocity *ctrl = new Controller::velocity(&pendulumAngle,&pendulumVelocity, &motorSpeed, &setAngle, kp, ki, kd, dir);
#else
	Controller::basic *ctrl = new Controller::basic(&pendulumAngle, &motorSpeed, &setAngle, kp, ki, kd, dir);
#endif

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
		std::cout << pendulumEQEP->getAngleDeg() << "\r" << std::flush;
	}

	// Set controller parameters
	ctrl->SetOutputLimits(-3200.0,3200.0);
	ctrl->SetSampleTime(20); // sample time in milliseconds
	ctrl->SetMode(1); // Automatic

	std::cout << ctrl->name() << " controller running ...." << std::endl;

	// Reset pendulum position to make vertical zero
	pendulumEQEP->setPosition(180-abs(pendulumEQEP->getAngleDeg()));
	motorEQEP->setPosition(0);

	// start the controller thread
	ctrl->run();
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
		motorAngle = motorEQEP->getAngle();
		motorAngleDeg = motorAngle * 180 / M_PI; // convert radian to degrees
		motorVelocity = motorEQEP->getVelocity();

		// Motor doesn't move unless speed > 350
		setSpeed = ( motorSpeed > 0 ? 1 : -1) * 350 + (int)motorSpeed;
		if (setSpeed > 3200) {
			setSpeed = 3200;
		} else if (setSpeed < -3200) {
			setSpeed = -3200;
		}

		std::cout << "setSpeed: " << setSpeed << "\r" << std::flush;

		// stop the motor if we deviate too far from vertical
		SMC->SetTargetSpeed( ((abs(pendulumAngleDeg) > 30) ? 0 : setSpeed) );

		lastTime = now;
		runTime = (now - start);
	} while (runTime.count() < 90);

	SMC->SetTargetSpeed(0);
	ctrl->stop();
	pendulumEQEP->stop();
	motorEQEP->stop();

	// Don't quit until all threads are finished
	WAIT_THREAD_FINISH(ctrl);
	WAIT_THREAD_FINISH(pendulumEQEP);
	WAIT_THREAD_FINISH(motorEQEP);

	delete ctrl;
	delete pendulumEQEP;
	delete motorEQEP;

	SMC->SetTargetSpeed(0);
	std::cout << "Done!" << std::endl;
	return;
}

int main(int argc, char const *argv[]) {
	std::vector<std::string> args(argv +1, argv + argc);

	std::cout << "Checking overlays are loaded... \n" << std::flush;

	if (checkOverlays()) {
		std::cout << "OK" << std::endl;
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
