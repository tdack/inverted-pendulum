/**
 * @file pid.cpp
 * @brief Threaded PID controller
 *
 * @author Troy Dack
 * @date Copyright (C) 2015
 *
 * @license
 * \verbinclude "Troy Dack GPL-2.0.txt"
 *
 **/

#include <pendulum.h>
#include <pid.h>
#include <cstdbool>
#include <iostream>
#include <atomic>

pid::pid(float _motor_voltage, float _k_p, float _k_i, float _k_d)
: motor_voltage(_motor_voltage), k_p(_k_p), k_i(_k_i), k_d(_k_d) {
	err_p = 0.0; // proportional error
	err_i = 0.0; // integral error
	err_d = 0.0; // derivative error
	motor_speed = 0.0;
	pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, ENCODER_PPR);
	motorEQEP = new threadedEQEP(MOTOR_EQEP, MOTOR_PPR);
	bExit.store(false);
}

pid::pid(float _motor_voltage, float _k_p, float _k_i, float _k_d, threadedEQEP *_pendulumEQEP, threadedEQEP *_motorEQEP)
: motor_voltage(_motor_voltage), k_p(_k_p), k_i(_k_i), k_d(_k_d),
  pendulumEQEP(_pendulumEQEP),
  motorEQEP(_motorEQEP) {
	err_p = 0.0; // proportional error
	err_i = 0.0; // integral error
	err_d = 0.0; // derivative error
	motor_speed = 0.0;
	bExit.store(false);
}


void pid::onStartHandler() {

	float u = 0.0;

	// Start threads to read eQEPs
	motorEQEP->run();
	motorEQEP->setPosition(0);
	pendulumEQEP->run();
	pendulumEQEP->setPosition(0); //start at zero

	// Create a Simple Motor Controller object
	Pololu::SMC *SMC = new Pololu::SMC(POLOLU_TTY);

	SMC->SetTargetSpeed(0);
	
	std::cout << "Starting PID control loop ..." << std::endl;
	while (!bExit.load()) {
		yield();
		err_p = 0 + pendulumEQEP->getAngle();
		err_d = 0 + pendulumEQEP->getVelocity();
		err_i = err_p + err_d;
		u = -((k_p * err_p) + (k_d * err_d) + (k_i * err_i));
		motor_speed = 100 / motor_voltage * u; // Calculate speed as a percentage

		if (motor_speed >= 100) {
			motor_speed = 100;
		} else if (motor_speed <= -100) {
			motor_speed= -100;
		}
		SMC->SetTargetSpeed(motor_speed);
//		std::cout << "\rp: " << err_p << " \ti: " << err_i << " \td:" << err_d << " \tu:" << u << " \tmotor_speed:" << motor_speed << std::endl;
		usleep(20); // Sleep a bit, give motor time to respond to changes
	}
	SMC->SetTargetSpeed(0);
}

void pid::stop() {
	motorEQEP->stop();
	pendulumEQEP->stop();
	bExit.store(true);
}

float pid::getKP() {
	return k_p;
}

void pid::setKP(float _k_p)
{
	k_p.store(_k_p);
	return;
}

float pid::getKI() {
	return k_i;
}

void pid::setKI(float _k_i)
{
	k_i.store(_k_i);
	return;
}

float pid::getKD() {
	return k_d;
}

void pid::setKD(float _k_d)
{
	k_d.store(_k_d);
	return;
}
