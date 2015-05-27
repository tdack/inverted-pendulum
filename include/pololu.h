/*
 * pololu.h
 *
 *  Created on: 27 May 2015
 *      Author: troy
 */

#ifndef INCLUDE_POLOLU_H_
#define INCLUDE_POLOLU_H_

#define MOTOR_EQEP 0
#define PENDULUM_EQEP 1
const double MOTOR_TEETH = 40.0;
const double ENCODER_TEETH = 12.0;
const double ENCODER_PPR = 1600.0;
const double ENCODER_RATIO = ENCODER_PPR * MOTOR_TEETH / ENCODER_TEETH;
const char* POLOLU_TTY = "/dev/ttyO1";

#endif /* INCLUDE_POLOLU_H_ */
