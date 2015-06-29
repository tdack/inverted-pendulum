/**
 *! @file pid.cpp
 *! Threaded PID controller
 *!
 *! @author troy
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

#include <pid.h>
#include <pololu.h>
#include <cstdbool>
#include <iostream>

pid::pid(float motor_voltage, float k_p, float k_i, float k_d) {
	this->motor_voltage = motor_voltage;
	this->k_p = k_p;
	this->k_i = k_i;
	this->k_d = k_d;
	err_p = 0.0; // proportional error
	err_i = 0.0; // integral error
	err_d = 0.0; // derivative error
	motor_speed = 0.0;
	pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, ENCODER_PPR);
	motorEQEP = new threadedEQEP(MOTOR_EQEP, MOTOR_PPR);
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
		//std::cout << "\rp: " << err_p << " \ti: " << err_i << " \td:" << err_d << " \tu:" << u << " \tmotor_speed:" << motor_speed << std::endl;
		usleep(20); // Sleep a bit, give motor time to respond to changes
	}
	SMC->SetTargetSpeed(0);
}

void pid::stop() {
	motorEQEP->stop();
	pendulumEQEP->stop();
	bExit.store(true);
}
