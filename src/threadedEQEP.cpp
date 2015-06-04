/*
 * threadedEQEP.cpp
 *
 *  Created on: 4 Jun 2015
 *      Author: troy
 */

#include <iostream>
#include <sys/time.h>
#include <math.h>
#include "bbb-eqep/bbb-eqep.h"
#include "BlackLib/BlackThread/BlackThread.h"
#include "threadedEQEP.h"

/**
 * threadedEQEP threaded class. Reads eQEP position information continuously
 *
 * @param __eqep eQEP # to use (0, 1, 2)
 * @param __ppr Pulses per revoultion for encoder
 */
threadedEQEP::threadedEQEP(int __eqep, float __ppr) {
	eqep = new BBB::eQEP(__eqep);
	eqep->resetPositionCounter();			// reset eQEP
	eqep->positionCounterSourceSelection(0); // set Quadrature mode
	eqep->enablePositionCompareShadow();		// enable Shadow
	eqep->setCaptureLatchMode(BBB::eQEP::CLMCPU);
	eqep->enableCaptureUnit();
	eqep->setUnitPeriod(1000);
	bExit = false;
	position.store(0);
	dt_position.store(0);
	velocity.store(0);
	ppr = __ppr;
}

/**
 * onStartHandler - main thread routine.
 *
 * Runs continuously until bExit is set to True
 */
void threadedEQEP::onStartHandler() {
	struct timeval old_time, new_time;
	int new_pos = 0;
	int old_pos = 0;
	float w = 0.0;
	float dt = 0.0;
	float v = 0.0;

	gettimeofday(&new_time,NULL);
	while (!this->bExit.load()) {
		yield(); // let other processes run if needed
		old_time = new_time;
		old_pos = new_pos;
		// Get new position
		new_pos = eqep->getPosition();
		gettimeofday(&new_time,NULL);
		// Time between last read and this one in seconds
		dt = ((1000000 * new_time.tv_sec + new_time.tv_usec)-(1000000 * old_time.tv_sec + old_time.tv_usec))/1e6;
		// Angle between last read and this read in radians
		w = (new_pos - old_pos)/ppr * 2.0 * M_PI;
		// Velocity for this period
		v = w/dt; // velocity in radians/second
		// Store new values
		position.store(new_pos);
		dt_position.store(new_pos - old_pos);
		velocity.store(v);
		// Need a small delay here to get velocity measurements
		usleep(20);
	}
	return;
}

void threadedEQEP::stop(){
	bExit.store(true);
	BlackThread::stop();
}

int threadedEQEP::getPosition() {
	return position.load();
}

float threadedEQEP::getAngle(){
	return position.load() / ppr * 2 * M_PI;
}

float threadedEQEP::getAngleDeg(){
	return getAngle() * 180 / M_PI;
}

float threadedEQEP::getVelocity(){
	return velocity.load();
}

float threadedEQEP::getVelocityDeg(){
	return getVelocity() * 180 / M_PI;
}

int threadedEQEP::getDeltaPosition(){
	return dt_position.load();
}
