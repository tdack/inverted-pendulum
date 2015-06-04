/*
 * readEQEP.h
 *
 *  Created on: 4 Jun 2015
 *      Author: troy
 */

#ifndef INCLUDE_READEQEP_H_
#define INCLUDE_READEQEP_H_

#include <atomic>
#include "bbb-eqep/bbb-eqep.h"
#include "BlackLib/BlackThread/BlackThread.h"

class readEQEP : public BlackLib::BlackThread {

private:

	BBB::eQEP *eqep;
	std::atomic<bool> bExit;
	std::atomic<int> position;
	std::atomic<int> dt_position;
	std::atomic<float> velocity;

public:

	readEQEP(int __eqep);

	void onStartHandler();

	void Exit();

	int getPosition();

	float getAngle();

	float getVelocity();

	int getDeltaPosition();

};



#endif /* INCLUDE_READEQEP_H_ */
