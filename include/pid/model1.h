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


#ifndef INCLUDE_PID_MODEL1_H_
#define INCLUDE_PID_MODEL1_H_

#include <BlackLib/BlackThread/BlackThread.h>
#include <atomic>
#include <cstdbool>

class threadedEQEP;

namespace PID {

class model1 : public BlackLib::BlackThread {

private:
	float motor_voltage; // input voltage to motor controller
	float k_p; 			 // proportional factor
	float k_i; 			 // integral factor
	float k_d; 			 // derivative factor
	float err_p; 		 // proportional error
	float err_i; 		 // integral error
	float err_d; 		 // derivative error
	double motor_speed;

	std::atomic<bool> bExit; // flag that thread should quit
	threadedEQEP *motorEQEP;
	threadedEQEP *pendulumEQEP;

public:

	/**
	 * Proportional-Integral-Derivative controller for inverted pendulum
	 *
	 * @param motor_voltage maxium voltage used to drive motor at full speed
	 * @param k_p		proportional control constant
	 * @param k_i		integral control constant
	 * @param k_d		derivative control constant
	 */
	model1(float motor_voltage, float k_p, float k_i, float k_d);

	model1(float _motor_voltage, float _k_p, float _k_i, float _k_d, threadedEQEP *_pendulumEQEP, threadedEQEP *_motorEQEP);

	void onStartHandler();

	void stop();
};
}; /* namespace PID*/
#endif /* INCLUDE_PID_MODEL1_H_ */
