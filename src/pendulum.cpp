/**
 * @file
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
#include <pendulum.h>
#include <pid.h>
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

	// Create new EQEPs object to monitor the pendulum & motor position
	threadedEQEP *pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, ENCODER_PPR);
	threadedEQEP *motorEQEP = new threadedEQEP(MOTOR_EQEP, MOTOR_PPR);

	// Initialise display
	BlackLib::BlackI2C *I2C_1 = new BlackLib::BlackI2C(BlackLib::I2C_1, 0x3c);
	SSD1306::SSD1306 oled(I2C_1, NULL, 64);
	oled.begin();
	SSD1306::gfx fx(oled);

	// Display initial message
	fx.clearScreen();
	fx.setTextColor(SSD1306::RGB::black, SSD1306::RGB::white);
	fx.setTextSize(1);
	fx.setCursor(0,0);
	fx.write("\n Raise the pendulum\n P:\n M:");
	fx.drawRoundRect(0,0,fx.getWidth(),fx.getHeight(), 8, SSD1306::RGB::black);
	// Start the thread running
	pendulumEQEP->run();

	// Wait until the pendulum is @ 180 +-1 deg
	// Assumes pendulum starts hanging vertically down
	while (abs(pendulumEQEP->getAngleDeg()) < 179 || abs(pendulumEQEP->getAngleDeg() > 181))  {
		fx.setCursor(18, 16);
		fx.write(to_string(pendulumEQEP->getAngleDeg()).c_str());
		fx.refreshScreen();
		usleep(20);
	}

	// Create a new PID controller thread
	pid *Controller = new pid(11.7, 10, 8, 40, pendulumEQEP, motorEQEP);

	Controller->run();

	// Let the thread run for a bit
	fx.setCursor(6,8);
	fx.write("PID Running ....  ");
	int count = 0;
	while (count < 50)  {
		count++;
		fx.setCursor(18,16);
		fx.write(to_string(pendulumEQEP->getAngleDeg()).c_str());
		fx.setCursor(18,24);
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

void testOLED() {
	BlackLib::BlackI2C *I2C_1 = new BlackLib::BlackI2C(BlackLib::I2C_1, 0x3c);
	SSD1306::SSD1306 oled(I2C_1, NULL, 64);

	int X = oled.get_width();
	int Y = oled.get_height();

	int max = (X > Y) ? X : Y;

	oled.begin();
	SSD1306::gfx fx(oled);

	fx.clearScreen();
	fx.setTextColor(SSD1306::RGB::black, SSD1306::RGB::white);
	fx.setTextSize(2.5);

	fx.setCursor(0,0);
	fx.write("Hello\nWorld!");

	sleep(2);
	oled.clear();
	fx.drawLine(0,0,X,Y, SSD1306::RGB::black);
	fx.drawLine(0,Y,X,0, SSD1306::RGB::black);
	for (int i = 0; i < max/5 -1; i += 10) {
		fx.drawRoundRect(i, i,(X - 2 * i),(Y - 2 * i),8, SSD1306::RGB::black);
	}
	fx.drawCircle(X/2,Y/2,max/4 - 1, SSD1306::RGB::black);
	oled.refresh();

	sleep(2);
}

int main(int argc, char const *argv[]) {

	if (!checkOverlays()) {
		rlutil::setColor(rlutil::WHITE);
		cout << "Are the overlays loaded?" << std::endl;
		return -1;
	}

	testOLED();
	motorTest();
	controller();
	return 0;
}
