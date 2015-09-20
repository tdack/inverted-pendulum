/**
 * @file
 * <Description>
 *
 * @author troy
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

#ifndef INCLUDE_SSD1306_OLED_H_
#define INCLUDE_SSD1306_OLED_H_

#include <atomic>
#include "BlackLib/BlackThread/BlackThread.h"
#include <BlackLib/BlackI2C/BlackI2C.h>
#include <SSD1306/gfx.h>
#include <SSD1306/ssd1306.h>
#include <queue>

namespace SSD1306 {

/*
 *
 */
class OLED : public BlackLib::BlackThread {

public:
	gfx *fx; // expose the gfx object so that things can be done when the thread isn't running

	OLED();
	virtual ~OLED();

	void onStartHandler();
	void write(int row, int col, std::string msg);

	void stop();

private:
	BlackLib::BlackI2C *I2C;
	SSD1306 *display;
	std::atomic<bool> bExit;	// Used to tell thread to exit

	struct msgStruct {
		int row;
		int col;
		std::string msg;
	};

	std::queue<msgStruct> displayQ;
};

} /* namespace SSD1306 */

#endif /* INCLUDE_SSD1306_OLED_H_ */
