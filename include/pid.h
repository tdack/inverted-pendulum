/**
 *! @file pid.h
 *! Threaded PID controller header file
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


#ifndef INCLUDE_PID_H_
#define INCLUDE_PID_H_

#include <atomic>
#include "pololu_serial.h"
#include "threadedEQEP.h"

class pid : public BlackLib::BlackThread {

private:
	float motor_voltage; // input voltage to motor controller
	float k_p; 			 // proportional factor
	float k_i; 			 // integral factor
	float k_d; 			 // derivative factor
	float err_p; 		 // proportional error
	float err_i; 		 // integral error
	float err_d; 		 // derivative error
	float motor_speed;

	std::atomic<bool> bExit; // flag that thread should quit
	threadedEQEP *motorEQEP;
	threadedEQEP *pendulumEQEP;
	Pololu::SMC *SMC;

public:

	/**
	 * Proportional-Integral-Derivative controller for inverted pendulum
	 *
	 * @param motor_voltage maxium voltage used to drive motor at full speed
	 * @param k_p		proportional control constant
	 * @param k_i		integral control constant
	 * @param k_d		derivative control constant
	 * @param pendulum	reference to a threadedEQEP object for the pendulum encoder
	 * @param motor		reference to a threadedEQEP object for the motor encoder
	 * @param smc		reference to a Pololu::SMC Simple Motor Controller object
	 */
	pid(float motor_voltage, float k_p, float k_i, float k_d, threadedEQEP *&pendulum, threadedEQEP *&motor, Pololu::SMC *&smc);

	void onStartHandler();

	void stop();
};

#endif /* INCLUDE_PID_H_ */
