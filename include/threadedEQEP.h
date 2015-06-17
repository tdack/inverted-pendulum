/**
 * @file
 *
 * @author Troy Dack
 * @date Copyright (C) 2015
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

#ifndef INCLUDE_threadedEQEP_H_
#define INCLUDE_threadedEQEP_H_

#include <atomic>
#include "bbb-eqep/bbb-eqep.h"
#include "BlackLib/BlackThread/BlackThread.h"

class threadedEQEP : public BlackLib::BlackThread {

private:

	BBB::eQEP *eqep;
	std::atomic<bool> bExit;
	std::atomic<int> position;
	std::atomic<int> dt_position;
	std::atomic<float> velocity;
	float ppr; // Pulse per revolution

public:

	threadedEQEP(int eqep_number, float encoder_ppr);

	void onStartHandler();

	void stop();

	int getPosition();

	float getAngle();

	float getAngleDeg();

	float getVelocity();

	float getVelocityDeg();

	int getDeltaPosition();

	void setPosition(uint32_t position);

};

#endif /* INCLUDE_threadedEQEP_H_ */
