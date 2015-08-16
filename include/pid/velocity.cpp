/**
 * @file velocity.cpp
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
#include <pololuSMC.h>
#include <threadedEQEP.h>
#include <iostream>
#include "velocity.h"


namespace PID {

velocity::velocity(double* Angle, double* Velocity, double* Output, double* SetPoint, double _kp,	double _ki, double _kd, int dir) :
		myAngle(Angle), myVelocity(Velocity), myOutput(Output), mySetPoint(SetPoint), inAuto(false), SampleTime(0.1) {
	bExit.store(false);
	SetOutputLimits(0, 100);
	SetControllerDirection(dir);
	SetTunings(_kp, _ki, _kd);
	lastTime = std::chrono::high_resolution_clock::now();
}

void velocity::Compute() {
	if (!inAuto)
		return;
	std::chrono::duration<float, std::deci> timeChange;
	std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
	timeChange = (now - lastTime);
	if (timeChange.count() >= SampleTime) {
		/*Compute all the working error variables*/
		double u = 0.0;

		double err_p = 0 - *myAngle;
		double err_d = 0 - *myVelocity;
		double err_i = err_p + err_d;
		u = -((kp * err_p) + (kd * err_d) + (ki * err_i));
		double output = outMax / 11.7 * u;

		if (output > outMax) {
			output = outMax;
		} else if (output < outMin) {
			output = outMin;
		}
		*myOutput = output;

		lastTime = now;
	}
}

void velocity::onStartHandler() {
	Initialize();
	while (!bExit.load()) {
		this->Compute();
		yield();
	}
}

void velocity::stop() {
	bExit.store(true);
}
/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void velocity::SetTunings(double Kp, double Ki, double Kd) {
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
void velocity::SetSampleTime(int NewSampleTime) {
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
void velocity::SetOutputLimits(double Min, double Max) {
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
void velocity::SetMode(int Mode) {
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
void velocity::Initialize() {
	// reset lastTime in case thread wasn't run straight after being created.
	lastTime = std::chrono::high_resolution_clock::now();
	ITerm = *myOutput;
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
void velocity::SetControllerDirection(int Direction) {
	if (inAuto && Direction != controllerDirection) {
		kp = (0 - kp);
		ki = (0 - ki);
		kd = (0 - kd);
	}
	controllerDirection = Direction;
}

}; /* namespace PID */
