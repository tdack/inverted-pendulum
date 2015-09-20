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

#include <SSD1306/OLED.h>

namespace SSD1306 {


OLED::OLED() {

	I2C = new BlackLib::BlackI2C(BlackLib::I2C_1, 0x3c);
	display = new SSD1306(I2C, NULL, 64);
	display->begin();
	fx = new gfx(*display);
}

OLED::~OLED() {
	// TODO Auto-generated destructor stub
}

/**
 * onStartHandler - main thread routine.
 *
 * Runs continuously until bExit is set to True
 */
void OLED::onStartHandler() {

	msgStruct msg;
	while (!bExit.load())
		if (!displayQ.empty()) {
			msg = displayQ.front();
			displayQ.pop();
			if (msg.row >= 0 && msg.col >= 0) {
				fx->setCursor(msg.row, msg.col);
			}
			fx->write(msg.msg.c_str());
		yield(); // let other processes run if needed
	}
	return;
}

void OLED::write(int row, int col, std::string msg) {
	msgStruct tmp;
	tmp.row = row;
	tmp.col = col;
	tmp.msg = msg;
	displayQ.push(tmp);
}


void OLED::stop(){
	bExit.store(true);
}

} /* namespace SSD1306 */
