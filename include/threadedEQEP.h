/*
 * threadedEQEP.h
 *
 *  Created on: 4 Jun 2015
 *      Author: troy
 */

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

	threadedEQEP(int __eqep, float __ratio);

	void onStartHandler();

	void stop();

	int getPosition();

	float getAngle();

	float getAngleDeg();

	float getVelocity();

	float getVelocityDeg();

	int getDeltaPosition();

};



#endif /* INCLUDE_threadedEQEP_H_ */
