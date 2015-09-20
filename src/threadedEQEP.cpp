/**
 *! @file threadedEQEP.cpp
 *! A threaded class to read an eQEP continuously.
 *!
 *! @author Troy Dack
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

#include <math.h>
#include <stddef.h>
#include <sys/time.h>
#include <threadedEQEP.h>
#include <cstdbool>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <thread>

/**
 * threadedEQEP threaded class. Reads eQEP position information continuously
 *
 * @param eqep_number eQEP # to use (0, 1, 2)
 * @param encoder_ppr Pulses per revoultion for encoder
 */
threadedEQEP::threadedEQEP(int eqep_number, double encoder_ppr)
	: bExit(0)
	, position(0)
	, dt_position(0)
	, velocity(0.0)
	, ppr(encoder_ppr)
{
	try {
		eqep = new BBB::eQEP(eqep_number);
	}
	catch (std::runtime_error& err) {
		throw std::runtime_error("Failed to access eQEP");
	}
	eqep->resetPositionCounter();			 // reset eQEP
	eqep->positionCounterSourceSelection(0); // set Quadrature mode
	eqep->enablePositionCompareShadow();	 // enable Shadow
	eqep->setCaptureLatchMode(BBB::eQEP::CLMCPU);
	eqep->enableCaptureUnit();
	eqep->setUnitPeriod(1000);
}

/**
 * onStartHandler - main thread routine.
 *
 * Runs continuously until bExit is set to True
 */
void threadedEQEP::onStartHandler() {

	int new_pos = 0;
	int old_pos = 0;
	double w = 0.0;
	double v = 0.0;

	std::chrono::duration<double, std::deci> dt;
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	while (!this->bExit.load()) {
		//yield(); // let other processes run if needed
		end = start;
		old_pos = new_pos;
		// Get new position
		new_pos = eqep->getPosition();
		start = std::chrono::high_resolution_clock::now();
		// Time between last read and this one in seconds
		dt = start - end;
		// Angle between last read and this read in radians
		w = (new_pos - old_pos)/ppr * 2.0 * M_PI;
		// Velocity for this period
		v = w/dt.count(); // velocity in radians/second
		// Store new values
		position.store(new_pos);
		dt_position.store(new_pos - old_pos);
		velocity.store(v);
		// Need a small delay here to get velocity measurements
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return;
}

void threadedEQEP::stop(){
	bExit.store(true);
}

int threadedEQEP::getPosition() {
	return position.load();
}

double threadedEQEP::getAngle(){
	return position.load() / ppr * 2 * M_PI;
}

double threadedEQEP::getAngleDeg(){
	return getAngle() * 180 / M_PI;
}

double threadedEQEP::getVelocity(){
	return velocity.load();
}

double threadedEQEP::getVelocityDeg(){
	return getVelocity() * 180 / M_PI;
}

int threadedEQEP::getDeltaPosition(){
	return dt_position.load();
}

void threadedEQEP::setPosition(uint32_t position) {
	eqep->setPosition(position);
	this->position.store(position);
}

void threadedEQEP::setDeg(double deg){
	int posn = int(ppr / 360 * deg);
	setPosition(posn);
}
