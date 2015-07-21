/**
 *! @file pololu.h
 *! System constants and defines
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

#ifndef INCLUDE_PENDULUM_H_
#define INCLUDE_PENDULUM_H_
#include <pid.h>
#include <pololu_serial.h>
#include <SSD1306/gfx.h>
#include <SSD1306/rgb_driver.h>
#include <SSD1306/ssd1306.h>
#include <threadedEQEP.h>
#include <rlutil.h>
#include <sys/stat.h>
#include <cmath>
#include <cstdbool>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>


/**
 * eQEP0 - Motor
 *
 * P9_42 = GPIO3_18 = EQEP0A_in, MODE1
 * P9_27 = GPIO3_19 = EQEP0B_in, MODE1
 **/
#define MOTOR_EQEP 1
/**
 * eQEP1 - Pendulum
 *
 * P8_33 = GPIO0_9 = EQEP1B_in, MODE2
 * P8_35 = GPIO0_8 = EQEP1A_in, MODE2
 **/
#define PENDULUM_EQEP 0

const double MOTOR_TEETH = 40.0;	// * Number of teeth on motor pulley
const double ENCODER_TEETH = 12.0;	// * Number of teeth on encoder pulley
const double MOTOR_PPR = 1600.0 * MOTOR_TEETH / ENCODER_TEETH;
const double PENDULUM_PPR = 1600.0;	// * pulses per revolution (x4 mode)
const char* const POLOLU_TTY = "/dev/ttyO2"; // * tty Pololu motor controller is on

#endif /* INCLUDE_PENDULUM_H_ */
