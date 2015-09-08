/**
 * @file lqr.cpp
 * @brief Threaded Linear Quadrature Regulator controller
 *
 * @author Troy Dack
 * @date Copyright (C) 2015
 *
 * @license
 * \verbinclude "Troy Dack GPL-2.0.txt"
 *
 **/

#include <Controller/lqr.h>
#include <pendulum.h>
#include <pololuSMC.h>
#include <threadedEQEP.h>
#include <iostream>


namespace Controller {

lqr::lqr(double* _pAngle, double* _pVelocity, double* _mAngle, double* _mVelocity, double* Output,
		 double* SetPoint, double _k1, double _k2, double _k3, double _k4, int dir) :
		pAngle(_pAngle), pVelocity(_pVelocity),
		mAngle(_mAngle), mVelocity(_mVelocity),
		myOutput(Output), mySetPoint(SetPoint),
		inAuto(false), SampleTime(20) {
	bExit.store(false);
	SetOutputLimits(0, 100);
	SetControllerDirection(dir);
	SetTunings(_k1, _k2, _k3, _k4);
	lastTime = std::chrono::high_resolution_clock::now();
}

void lqr::Compute() {
	if (!inAuto)
		return;
	std::chrono::duration<float, std::deci> timeChange;
	std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
	timeChange = (now - lastTime);
	if (timeChange.count() >= SampleTime) {
		/*Compute all the working error variables*/
		double pA = *pAngle;
		double pV = *pVelocity;
		double mA = *mAngle;
		double mV = *mVelocity;
		double u = ( (k1 * pA) + (k2 * mA) + (k3 * pV) + (k4 * mV) );

		double output = outMax / 11.7 * u;

		if (output > outMax) {
			output = outMax;
		} else if (output < outMin) {
			output = outMin;
		}
		std::cout << "lqr: " << pA << " " << pV  << " " << mA << " " << mV << " " << u << "  dt: " << timeChange.count() << std::endl;
		*myOutput = output;

		lastTime = now;
	}
}

void lqr::onStartHandler() {
	Initialize();
	while (!bExit.load()) {
		this->Compute();
		yield();
	}
}

void lqr::stop() {
	bExit.store(true);
}
/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted.
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/
void lqr::SetTunings(double _k1, double _k2, double _k3, double _k4) {

	k1 = dispK1 = _k1;
	k2 = dispK2 = _k2;
	k3 = dispK3 = _k3;
	k4 = dispK4 = _k4;
}

/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed
 ******************************************************************************/
void lqr::SetSampleTime(int NewSampleTime) {
	if (NewSampleTime > 0) {
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
void lqr::SetOutputLimits(double Min, double Max) {
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
void lqr::SetMode(int Mode) {
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
void lqr::Initialize() {
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
 * The LQR will either be connected to a DIRECT acting process (+Output leads
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void lqr::SetControllerDirection(int Direction) {
	controllerDirection = Direction;
}

}; /* namespace CONTROLLER */
