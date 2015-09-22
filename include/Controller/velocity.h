/**
 *! @file velocity.h
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


#ifndef INCLUDE_CONTROLLER_VELOCITY_H_
#define INCLUDE_CONTROLLER_VELOCITY_H_

//#define DEBUG

#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif

#include <BlackLib/BlackThread/BlackThread.h>
#include <atomic>
#include <cstdbool>
#include <chrono>

namespace Controller {
class velocity : public BlackLib::BlackThread {

public:

	/**
	 * @brief Proportional-Integral-Derivative controller for inverted pendulum
	 *
	 *@description Uses pendulum angle and velocity to determine motor speed and direction
	 *
	 * @param[in] Angle	angle of the pendulum
	 * @param[in] Velocity velocity of the pendulum
	 * @param[out] Output motor speed
	 * @param[in] SetPoint target for controller
	 * @param[in] _kp Proportional constant
	 * @param[in] _ki Integral constant
	 * @param[in] _kd Derivative constant
	 * @param[in] dir Direction controller is to operate in. 0=normal, 1=inverse
	 */
	velocity(double* Angle, double* Velocity, double* Output, double* SetPoint, double _kp,	double _ki, double _kd, int dir);

	/*!
	 * @brief Set Controller operating mode
	 * @description In auto mode the controller will alter the Output to drive towards
	 * 				the SetPoint.
	 * 				In manual mode the controller is disabled and control of the Output
	 * 				is handled external to the controller
	 *
	 * @param[in] Mode controller mode 0=manual, 1=auto
	 */
	void SetMode(int Mode); // * sets pid-new to either Manual (0) or Auto (non-0)

	/*!
	 * @brief Set output limits for controller
	 * @description Clamps the output to a specific range. 0-255 by default, but
	 *				it's likely the user will want to change this depending on
	 * 				the application
	 *
	 * @param[in] Min Minimum limit
	 * @param[out] Max Maximum limit
	 */
	void SetOutputLimits(double Min, double Max);

	/*!
	 * @brief Alter the PID parameters
	 * @description While most users will set the tunings once in the
	 *				constructor, this function gives the user the option
	 *				of changing tunings during runtime for Adaptive control
	 *
	 * @param[in] kp Proportional constant
	 * @param[in] ki Integral constant
	 * @param[in] kd Derivative constant
	 */
	void SetTunings(double kp, double ki, double kd);

	/*!
	 * @brief Set controller direction
	 * @description Sets the Direction, or "Action" of the controller. DIRECT
	 *				means the output will increase when error is positive. REVERSE
	 *				means the opposite.  it's very unlikely that this will be needed
	 *				once it is set in the constructor.
	 *
	 * @param[in] direction
	 */
	void SetControllerDirection(int);

	/*!
	 * @brief Set sample time for PID calculations
	 * @description Sets the frequency, in milliseconds, with which
	 *				the pid-new calculation is performed.  Default is 100
	 *
	 * @param[in] frequency in milliseconds
	 */
	void SetSampleTime(int);

	/*!
	 * @brief Get the value of the Proportional constant
	 * @return
	 */
	inline double GetKp() {
		return dispKp;
	}

	/*!
	 * @brief Get the value of the Integral constant
	 * @return
	 */
	inline double GetKi() {
		return dispKi;
	}

	/*!
	 * @brief Get the value of the Derivativeconstant
	 * @return
	 */
	inline double GetKd() {
		return dispKd;
	}

	/*!
	 * @brief Get the operating mode of the controller
	 * @return
	 */
	inline int GetMode() {
		return inAuto ? 1 : 0;
	}

	/*!
	 * @brief Get the direction of the controller
	 * @return
	 */
	inline int GetDirection() {
		return controllerDirection;
	}

    /*!
     * @brief Thread's start handler function.
	 */
	void onStartHandler();

	/*!
	 * @brief Stops the thread running
	 */
	void stop();

private:
	/*!
	 * @brief Initialise all parameters to ensure smooth transfer from manul to automatic
	 */
	void Initialize();

	/*!
	 * @brief Main loop that handle PID calculations
	 */
	void Compute();

	std::atomic<bool> bExit; 	/*!< flag to tell thread to quit */

	double dispKp;
	double dispKi;
	double dispKd;

	double kp;                  /*!< (P)roportional Tuning Parameter */
	double ki;                  /*!< (I)ntegral Tuning Parameter */
	double kd;                  /*!< (D)erivative Tuning Parameter */

	int controllerDirection;

	double *myAngle;  // * Pointers to the Input, Output, and Setpoint variables
	double *myVelocity;
	double *myOutput; //   This creates a hard link between the variables and the
	double *mySetPoint; //   pid, freeing the user from having to constantly tell us
						//   what these values are.  with pointers we'll just know.

	std::chrono::high_resolution_clock::time_point lastTime;
	double ITerm, lastInput;

	bool inAuto;
	double SampleTime;
	double outMin, outMax;
}; /* class VELOCITY */
}; /* namespace CONTROLLER */
#endif /* INCLUDE_CONTROLLER_VELOCITY_H_ */
