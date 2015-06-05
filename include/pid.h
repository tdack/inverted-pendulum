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

public:

	pid(float motor_voltage, float k_p, float k_i, float k_d);

	void onStartHandler();

	void stop();
};

#endif /* INCLUDE_PID_H_ */
