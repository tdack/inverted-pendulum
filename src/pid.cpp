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
#include <iostream>
#include "pid.h"
#include "pololu_serial.h"
#include "pololu.h"

pid::pid(float motor_voltage, float k_p, float k_i, float k_d) {
	this->motor_voltage = motor_voltage;
	this->k_p = k_p;
	this->k_i = k_i;
	this->k_d = k_d;
	err_p = 0.0; // proportional error
	err_i = 0.0; // integral error
	err_d = 0.0; // derivative error
	motor_speed = 0.0;
	motorEQEP = new threadedEQEP(MOTOR_EQEP, MOTOR_PPR);
	pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, ENCODER_PPR);
	bExit.store(false);
}

void pid::onStartHandler() {

	float u = 0.0;

	// Start threads to read eQEPs
	motorEQEP->run();
	pendulumEQEP->run();
	pendulumEQEP->setPosition(0); //start at zero
	
	// Simple Motor Controller object
	Pololu::SMC *SMC = new Pololu::SMC(POLOLU_TTY);

	std::cout << "Starting PID control loop ..." << std::endl;
	while (!bExit.load()) {
		yield();
		err_p = pendulumEQEP->getAngle() - 0;
		err_d = pendulumEQEP->getVelocity() - 0;
		err_i = err_p + err_d;
		u = -((k_p * err_p) + (k_d * err_d) + (k_i * err_i));
		motor_speed = 100 / motor_voltage * u; // Calculate speed as a percentage

		if (motor_speed >= 100) {
			motor_speed = 100;
		} else if (motor_speed <= -100) {
			motor_speed= -100;
		}
		SMC->SetTargetSpeed(motor_speed);
		std::cout << "\rp: " << err_p << "  i: " << err_i << "  d:" << err_d << "  u:" << u << "  motor_speed:" << motor_speed << "      ";
	}
	SMC->SetTargetSpeed(0);
	WAIT_THREAD_FINISH(motorEQEP);
	WAIT_THREAD_FINISH(pendulumEQEP);
}

void pid::stop() {
	motorEQEP->stop();
	pendulumEQEP->stop();
	bExit.store(true);
}
