/**
 * @file pololu.cpp
 * @brief Inverted Pendulum main file
 *
 * @author Troy Dack
 * @date Copyright (C) 2015
 *
 * @license
 * \verbinclude "Troy Dack GPL-2.0.txt"
 *
 **/

#include <BlackLib/BlackDef.h>
#include <BlackLib/BlackGPIO/BlackGPIO.h>
#include <BlackLib/BlackI2C/BlackI2C.h>
#include <BlackLib/BlackThread/BlackThread.h>
#include <pid.h>
#include <pololu.h>
#include <pololu_serial.h>
#include <rlutil.h>
#include <sys/stat.h>
#include <SSD1306/gfx.h>
#include <SSD1306/rgb_driver.h>
#include <SSD1306/ssd1306.h>
#include <threadedEQEP.h>
#include <unistd.h>
#include <cmath>
#include <cstdbool>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void controller() {

	// Create new EQEPs object to monitor the pendulum position
	threadedEQEP *pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, ENCODER_PPR);
	threadedEQEP *motorEQEP = new threadedEQEP(MOTOR_EQEP, MOTOR_PPR);

	// Initialise display
	BlackLib::BlackI2C *I2C_1 = new BlackLib::BlackI2C(BlackLib::I2C_1, 0x3c);
	SSD1306::SSD1306 oled(I2C_1, NULL, 64);
	oled.begin();
	SSD1306::gfx fx(oled);

	fx.clearScreen();
	fx.setTextColor(SSD1306::RGB::black, SSD1306::RGB::white);
	fx.setTextSize(1);
	fx.setCursor(1,1);
	fx.write("Raise the pendulum");

	// Start the thread running
	pendulumEQEP->run();

	rlutil::cls();
	rlutil::setColor(rlutil::BLUE);
	cout << "Raise the pendulum" << endl;
	rlutil::setColor(rlutil::WHITE);

	// Wait until the pendulum is @ 180 +-1 deg
	// Assumes pendulum starts hanging vertically down
	while (abs(pendulumEQEP->getAngleDeg()) < 179 || abs(pendulumEQEP->getAngleDeg() > 181))  {
		fx.setCursor(1,10);
		fx.write(to_string(pendulumEQEP->getAngleDeg()).c_str());
		fx.refreshScreen();
		usleep(20);
	}

	// Create a new PID controller thread
	pid *Controller = new pid(11.7, 10, 8, 40, pendulumEQEP, motorEQEP);

	Controller->run();

	// Let the thread run for a bit
	fx.setCursor(1,1);
	fx.write("                   ");
	int count = 0;
	while (count < 50)  {
		count++;
		fx.setCursor(1,10);
		fx.write(to_string(pendulumEQEP->getAngleDeg()).c_str());
		fx.setCursor(1,20);
		fx.write(to_string(motorEQEP->getAngleDeg()).c_str());
		fx.setCursor(110,54);
		fx.write(to_string(count).c_str());
		fx.refreshScreen();
	}

	Controller->stop();
	pendulumEQEP->stop();
	motorEQEP->stop();

	// Don't quit until all threads are finished
	WAIT_THREAD_FINISH(Controller);
	WAIT_THREAD_FINISH(pendulumEQEP);
	WAIT_THREAD_FINISH(motorEQEP);

	cout << "Done!" << endl;
	return;
}

bool checkOverlays(){
	std::vector<std::string> files = {
			POLOLU_TTY,								   // tty device path
			"/sys/bus/platform/devices/48300180.eqep", // eqep device path
			"/sys/bus/platform/devices/48302180.eqep"  // eqep device path
	};
	struct stat buffer;
	bool overlays_loaded = true;

	rlutil::cls();
	rlutil::setColor(rlutil::RED);
	for (std::string& file : files) {
		if (stat(file.c_str(), &buffer) != 0) {
			rlutil::setColor(rlutil::YELLOW);
			cout << file << " ";
			rlutil::setColor(rlutil::RED);
			cout << "not found." << endl;;
			overlays_loaded = false;
		}
	}
	return overlays_loaded;
}

void motorTest() {
	BlackLib::BlackGPIO P8_7(BlackLib::GPIO_66, BlackLib::output);
	BlackLib::BlackGPIO P8_8(BlackLib::GPIO_67, BlackLib::output);

	P8_7 << BlackLib::high;
	P8_8 << BlackLib::high;

	// Create a Simple Motor Controller object
	Pololu::SMC *SMC = new Pololu::SMC(POLOLU_TTY);

	SMC->SetTargetSpeed(0);

	cout.setf(std::ios::fixed);
	cout << "Voltage: " << std::setprecision(2) << SMC->GetVariable(23)/1e3 << endl;

	usleep(500);
	SMC->SetTargetSpeed(256);
	P8_7 << BlackLib::low;
	P8_8 << BlackLib::high;

	sleep(3);
	SMC->SetTargetSpeed(-256);
	P8_7 << BlackLib::high;
	P8_8 << BlackLib::low;

	sleep(3);
	SMC->SetTargetSpeed(0);

	P8_7 << BlackLib::low;
	P8_8 << BlackLib::low;

	delete SMC;
	return;
}

int main(int argc, char const *argv[]) {

	if (!checkOverlays()) {
		rlutil::setColor(rlutil::WHITE);
		cout << "Are the overlays loaded?" << std::endl;
		return -1;
	}

	controller();
	return 0;

	BlackLib::BlackI2C *I2C_1 = new BlackLib::BlackI2C(BlackLib::I2C_1, 0x3c);
	SSD1306::SSD1306 oled(I2C_1, NULL, 64);

	int X = oled.get_width();
	int Y = oled.get_height();

	int max = (X > Y) ? X : Y;

	float x, y;
	float dx = (X * 1.0) / max;
	float dy = (Y * 1.0) / max;

	oled.begin();

	SSD1306::gfx fx(oled);

	fx.clearScreen();
	fx.setTextColor(SSD1306::RGB::black, SSD1306::RGB::white);
	fx.setTextSize(2);

	fx.setCursor(10,10);
	fx.write("Hello World!");

	sleep(2);
	oled.clear();
	for (x = 0, y = 0; x < X; x += dx, y += dy) {
		oled.drawPixel((int) x, (int) y, SSD1306::RGB::black);
		oled.drawPixel((int) (oled.get_width() - x), (int) y,
				SSD1306::RGB::black);
	}
	oled.refresh();

	sleep(2);

	// Flash a medium square in the middle
	SSD1306::rgb_t color;

	X = oled.get_width() / 2 - 10;
	Y = oled.get_height() / 2 - 10;
	for (int i = 0; i < 51; i++) {
		color = (i % 2) ? SSD1306::RGB::white : SSD1306::RGB::black;
		for (x = 0; x < 20; x++) {
			for (y = 0; y < 20; y++) {
				oled.drawPixel(X + x, Y + y, color);
			}
		}
		oled.refresh();
	}

	// Flash a smaller square in the middle
	X = oled.get_width() / 2 - 4;
	Y = oled.get_height() / 2 - 4;
	for (int i = 0; i < 20; i++) {
		color = (i % 2) ? SSD1306::RGB::white : SSD1306::RGB::black;
		for (x = 0; x < 8; x++) {
			for (y = 0; y < 8; y++) {
				oled.drawPixel(X + x, Y + y, color);
			}
		}
		oled.refresh();
	}
	sleep(1);

	oled.clear();
	oled.refresh();
	oled.reset();

	return 0;
}
