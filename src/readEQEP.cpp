/*
 * readEQEP.cpp
 *
 *  Created on: 4 Jun 2015
 *      Author: troy
 */

#include <iostream>
#include <sys/time.h>
#include <math.h>
#include "bbb-eqep/bbb-eqep.h"
#include "BlackLib/BlackThread/BlackThread.h"
#include "readEQEP.h"

readEQEP::readEQEP(int __eqep) {
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
}

void readEQEP::onStartHandler() {
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
		w = (new_pos - old_pos)/(1600.0 * 40.0 / 12.0) * 2 * M_PI;
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

void readEQEP::Exit(){
	bExit.store(true);
}

int readEQEP::getPosition() {
	return position.load();
}

float readEQEP::getAngle(){
	return position.load() / (1600.0*40.0/12.0) * 2 * M_PI;
}
float readEQEP::getVelocity(){
	return velocity.load();
}
int readEQEP::getDeltaPosition(){
	return dt_position.load();
}
