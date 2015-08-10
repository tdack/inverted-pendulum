/**
 * @file basic.cpp
 * @brief Threaded PID controller
 *
 * Threaded PID controller based on https://github.com/br3ttb/Arduino-PID-Library
 *
 * @author Troy Dack
 * @date Copyright (C) 2015
 *
 * @license
 * \verbinclude "Troy Dack GPL-2.0.txt"
 *
 **/

#include <pid/basic.h>
#include <iostream>
#include <ratio>
#include <string>

namespace PID {
basic::basic(double* Input, double* Output, double* SetPoint, double _kp,
		double _ki, double _kd, int dir) :
		myInput(Input), myOutput(Output), mySetPoint(SetPoint), inAuto(false), SampleTime(0.1) {
	bExit.store(false);
	SetOutputLimits(0, 100);
	SetControllerDirection(dir);
	SetTunings(_kp, _ki, _kd);
	lastTime = std::chrono::high_resolution_clock::now();
}

void basic::Compute() {
	if (!inAuto)
		return;
	std::chrono::duration<float, std::deci> timeChange;
	std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
	timeChange = (now - lastTime);
	if (timeChange.count() >= SampleTime) {
		/*Compute all the working error variables*/
		double input = *myInput;
		double error = *mySetPoint - input;
		ITerm += (ki * error);
		if (ITerm > outMax) {
			ITerm = outMax;
		} else if (ITerm < outMin) {
			ITerm = outMin;
		}
		double dInput = (input - lastInput);

		/*Compute PID Output*/
		double output = kp * error + ITerm - kd * dInput;

		if (output > outMax) {
			output = outMax;
		} else if (output < outMin) {
			output = outMin;
		}
		*myOutput = output;

		/*Remember some variables for next time*/
		lastInput = input;
		lastTime = now;
//		std::cout << timeChange.count();
//		std::cout << "\t Input: " << std::to_string(input) << "\tError: "
//				<< std::to_string(error) << "\t";
//		std::cout << "Output: " << std::to_string(*myOutput) << std::endl;
	}
	return;
}

void basic::onStartHandler() {
	Initialize();
	while (!bExit.load()) {
		this->Compute();
		yield();
	}
}

void basic::stop() {
	bExit.store(true);
}

/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void basic::SetTunings(double Kp, double Ki, double Kd) {
	if (Kp < 0 || Ki < 0 || Kd < 0)
		return;

	dispKp = Kp;
	dispKi = Ki;
	dispKd = Kd;

	double SampleTimeInSec = ((double) SampleTime) / 1000;
	kp = Kp;
	ki = Ki * SampleTimeInSec;
	kd = Kd / SampleTimeInSec;

	if (controllerDirection == 1) {
		kp = (0 - kp);
		ki = (0 - ki);
		kd = (0 - kd);
	}
}

/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed
 ******************************************************************************/
void basic::SetSampleTime(int NewSampleTime) {
	if (NewSampleTime > 0) {
		double ratio = (double) NewSampleTime / 1000 / (double) SampleTime;
		ki *= ratio;
		kd /= ratio;
		SampleTime = (double) NewSampleTime / 1000.0;
	}
}

/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void basic::SetOutputLimits(double Min, double Max) {
	if (Min >= Max)
		return;
	outMin = Min;
	outMax = Max;

	if (inAuto) {
		if (*myOutput > outMax) {
			*myOutput = outMax;
		} else if (*myOutput < outMin) {
			*myOutput = outMin;
		}
		if (ITerm > outMax) {
			ITerm = outMax;
		} else if (ITerm < outMin) {
			ITerm = outMin;
		}
	}
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/
void basic::SetMode(int Mode) {
	bool newAuto = (Mode == 1);
	if (newAuto == !inAuto) { /*we just went from manual to auto*/
		Initialize();
	}
	inAuto = newAuto;
}

/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/
void basic::Initialize() {
	// reset lastTime in case thread wasn't run straight after being created.
	lastTime = std::chrono::high_resolution_clock::now();
	ITerm = *myOutput;
	lastInput = *myInput;
	if (ITerm > outMax) {
		ITerm = outMax;
	} else if (ITerm < outMin) {
		ITerm = outMin;
	}
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void basic::SetControllerDirection(int Direction) {
	if (inAuto && Direction != controllerDirection) {
		kp = (0 - kp);
		ki = (0 - ki);
		kd = (0 - kd);
	}
	controllerDirection = Direction;
}

};
/* namespace PID */
