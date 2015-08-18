/**
 *! @file lqr.h
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


#ifndef INCLUDE_CONTROLLER_LQR_H_
#define INCLUDE_CONTROLLER_LQR_H_

#include <BlackLib/BlackThread/BlackThread.h>
#include <atomic>
#include <cstdbool>
#include <chrono>

namespace Controller {

class lqr : public BlackLib::BlackThread {

public:

	/**
	 * Proportional-Integral-Derivative controller for inverted pendulum
	 *
	 *	Uses pendulum angle and lqr
	 */
	lqr(double* PendulumAngle, double* PendulumVelocity, double* MotorAngle, double* MotorVelocity, double* Output, double* SetPoint, double _k1,	double _k2, double _k3, double _k4, int dir);

	void SetMode(int Mode); // * sets pid-new to either Manual (0) or Auto (non-0)

	void SetOutputLimits(double Min, double Max); //clamps the output to a specific range. 0-255 by default, but
												  //it's likely the user will want to change this depending on
												  //the application

// available but not commonly used functions ********************************************************
	void SetTunings(double k1, double k2, double k3, double k4);

	void SetControllerDirection(int);// * Sets the Direction, or "Action" of the controller. DIRECT
									 //   means the output will increase when error is positive. REVERSE
									 //   means the opposite.  it's very unlikely that this will be needed
									 //   once it is set in the constructor.

	void SetSampleTime(int); // * sets the frequency, in Milliseconds, with which
							 //   the pid-new calculation is performed.  default is 100

	/* Status Funcions*************************************************************
	 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
	 * functions query the internal state of the PID.  they're here for display
	 * purposes.  this are the functions the PID Front-end uses for example
	 ******************************************************************************/
	inline double GetK1() {
		return dispK1;
	}
	inline double GetK2() {
		return dispK2;
	}
	inline double GetK3() {
		return dispK3;
	}
	inline double GetK4() {
		return dispK4;
	}
	inline int GetMode() {
		return inAuto ? 1 : 0;
	}
	inline int GetDirection() {
		return controllerDirection;
	}

	void onStartHandler();  // called by run() to do the work in the thread

	void stop();			// stops the thread running.

private:
	void Initialize();
	void Compute(); // does the actual PID calculations

	std::atomic<bool> bExit; 	// flag to tell thread to quit

	double dispK1;				// * we'll hold on to the tuning parameters in user-entered
	double dispK2;				//   format for display purposes
	double dispK3;				//
	double dispK4;				//

	double k1;
	double k2;
	double k3;
	double k4;

	int controllerDirection;

	double *pAngle;  // * Pendulum angle & velocity
	double *pVelocity;
	double *mAngle;  // * Motor angle & velocity
	double *mVelocity;
	double *myOutput; //   This creates a hard link between the variables and the
	double *mySetPoint; //   pid, freeing the user from having to constantly tell us
						//   what these values are.  with pointers we'll just know.

	std::chrono::high_resolution_clock::time_point lastTime;
	double ITerm, lastInput;

	bool inAuto;
	double SampleTime;
	double outMin, outMax;
};
}; /* namespace CONTROLLER */
#endif /* INCLUDE_CONTROLLER_LQR_H_ */
