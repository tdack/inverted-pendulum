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

#include <pendulum.h>

#include <Controller/basic.h>
#include <Controller/velocity.h>
#include <Controller/lqr.h>

using namespace std;

void controller(double kp, double ki, double kd, int dir) {

	// Initialise display
	SSD1306::OLED *outLED = new SSD1306::OLED();

	// Display initial message
	outLED->fx->clearScreen();
	outLED->fx->setTextColor(SSD1306::RGB::black, SSD1306::RGB::white);
	outLED->fx->setTextSize(1);
	outLED->fx->setCursor(2,4);
	outLED->fx->write(" Raise the pendulum");
	outLED->fx->setCursor(0, 24);
	outLED->fx->write(" P:\n M:\n S:");
	outLED->fx->drawRoundRect(0,0,outLED->fx->getWidth(), 16, 4, SSD1306::RGB::black);
	std::cout << "Raise the pendulum" << std::endl;

	// Variables that will be used to pass data to/from controller
	double pendulumAngle = 0;
	double pendulumVelocity =0;
	double motorAngle = 0;
	double motorVelocity =0;
	double motorSpeed = 0;
	double setAngle = 0;
	int count = 0;
	int setSpeed;

	// Measure time in processing loop
	std::chrono::high_resolution_clock::time_point lastTime;
	std::chrono::duration<float, std::deci> timeChange;
	std::chrono::high_resolution_clock::time_point now;
	now = std::chrono::high_resolution_clock::now();
	timeChange = (now - lastTime);

	// Create new EQEPs object to monitor the pendulum & motor position
	threadedEQEP *pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, ENCODER_PPR);
	threadedEQEP *motorEQEP = new threadedEQEP(MOTOR_EQEP, MOTOR_PPR);

	// Create a new controller
	Controller::basic *ctrl = new Controller::basic(&pendulumAngle, &motorSpeed, &setAngle, kp, ki, kd, dir);
//	Controller::velocity *ctrl = new Controller::velocity(&pendulumAngle,&pendulumVelocity, &motorSpeed, &setAngle, kp, ki, kd, dir);
//	Controller::lqr *ctrl = new Controller::lqr(&pendulumAngle, &pendulumVelocity,
//										&motorAngle, &motorVelocity,
//										&motorSpeed, &setAngle,
//										-23.1455, 126.3112, -5.7435, 7.5213, // LQR constants
//										dir);

	// Create a Simple Motor Controller object
	Pololu::SMC *SMC = new Pololu::SMC(POLOLU_TTY);
	// Stop the motor
	SMC->SetTargetSpeed(0);

	// Start the EQEP threads running
	pendulumEQEP->run();
	motorEQEP->run();

	// Wait until the pendulum is @ 180 +-1 deg
	// Assumes pendulum starts hanging vertically down
	while (abs(pendulumEQEP->getAngleDeg()) < 179 || abs(pendulumEQEP->getAngleDeg() > 181))  {
		outLED->fx->setCursor(18, 24);
		outLED->fx->write(to_string(pendulumEQEP->getAngleDeg()).c_str());
		outLED->fx->refreshScreen();
	}

	// Set controller parameters
	ctrl->SetOutputLimits(-3200.0,3200.0);
	ctrl->SetSampleTime(15); // sample time in milliseconds
	ctrl->SetMode(1); // Automatic


	// Reset pendulum position to make vertical zero
	pendulumEQEP->setPosition(180-abs(pendulumEQEP->getAngleDeg()));
	motorEQEP->setPosition(0);

	// start the controller thread
	ctrl->run();

	outLED->fx->setCursor(2,4);
	outLED->fx->write("Controller Running ");
	std::cout << "Controller Running ...." << std::endl;
	lastTime = std::chrono::high_resolution_clock::now();
	// Let the threads run for a bit
	while (count < 1500)  {
		now = std::chrono::high_resolution_clock::now();
		timeChange = (now - lastTime);
		pendulumAngle = pendulumEQEP->getAngleDeg();
		pendulumVelocity = pendulumEQEP->getVelocityDeg();
		motorAngle = motorEQEP->getAngleDeg();
		motorVelocity = motorEQEP->getVelocityDeg();
		// Motor doesn't move unless speed > 160
		setSpeed = ( motorSpeed > 0 ? 1 : -1) * 280 + (int)motorSpeed;
//		if (abs(pendulumAngle * 180 / M_PI) > 25) { // convert radian to degrees
		if (abs(pendulumAngle) > 25) {
			// stop the motor if we have deviated too far from vertical
			SMC->SetTargetSpeed(0);
		} else {
			SMC->SetTargetSpeed(setSpeed);
		}
		count++;

		outLED->write(18,24, to_string(pendulumEQEP->getAngleDeg()).c_str());
		outLED->write(18,32, to_string(motorEQEP->getAngleDeg()).c_str());
		outLED->write(18,40, to_string(setSpeed).c_str());
		outLED->write(-1,-1, "   ");
		outLED->write(110,54, to_string(count).c_str());
		outLED->write(-1, -1, "  ");
		outLED->run();
		lastTime = now;
	}
	SMC->SetTargetSpeed(0);

	outLED->stop();
	ctrl->stop();
	pendulumEQEP->stop();
	motorEQEP->stop();

	// Don't quit until all threads are finished
	WAIT_THREAD_FINISH(outLED);
	WAIT_THREAD_FINISH(ctrl);
	WAIT_THREAD_FINISH(pendulumEQEP);
	WAIT_THREAD_FINISH(motorEQEP);

	delete ctrl;
	delete pendulumEQEP;
	delete motorEQEP;

	cout << "Done!" << endl;
	return;
}

bool checkOverlays(){
	std::map<std::string, std::vector<std::string> > overlay_devices {
		{POLOLU_TTY, { "ADAFRUIT-UART2" } },		// /dev/ttyO2
		{"/sys/bus/platform/devices/48300180.eqep",	// eqep device path
					{ "PyBBIO-epwmss0",				// Enhanced PWM Sub System 0
					  "PyBBIO-eqep0" }				// EQEP 0
		},
		{"/sys/bus/platform/devices/48302180.eqep",	// eqep device path
					{ "PyBBIO-epwmss1",				// Enhanced PWM Sub System 1
					  "PyBBIO-eqep1" }				// EQEP 1
		}
	};
	struct stat buffer;
	bool overlays_loaded = true;
	string SLOTS = "/sys/devices/bone_capemgr.9/slots"; // Path to Cape Manager slots file
	ofstream fSlots;

	fSlots.open(SLOTS);
	if (!fSlots.is_open()) {
		cout << "Couldn't open " << SLOTS << ", can't load overlays." << std::endl;
		return false;
	}

	// Iterate over devices we need
	for (auto &dev : overlay_devices) {
		rlutil::setColor(rlutil::YELLOW);
		cout << dev.first << " ";
		// Check if file exists
		if (stat(dev.first.c_str(), &buffer) != 0) {
			rlutil::setColor(rlutil::YELLOW);
			cout << dev.first << " ";
			rlutil::setColor(rlutil::RED);
			cout << "not found .... ";
			// Load the overlays for this device
			for (auto &f : dev.second ) {
				fSlots << f.c_str() << std::flush;
			}
			rlutil::setColor(rlutil::GREEN);
			cout << "loaded!" << std::endl;
		} else {
			cout << std::endl;
		}
	}
	rlutil::setColor(rlutil::WHITE);
	fSlots.close();

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
	SMC->SetTargetSpeed(512);
	P8_7 << BlackLib::low;
	P8_8 << BlackLib::high;

	sleep(3);
	SMC->SetTargetSpeed(-512);
	P8_7 << BlackLib::high;
	P8_8 << BlackLib::low;

	sleep(3);
	SMC->SetTargetSpeed(0);

	P8_7 << BlackLib::low;
	P8_8 << BlackLib::low;

	cout << "Voltage: " << std::setprecision(2) << SMC->GetVariable(23)/1e3 << endl;

	delete SMC;
	return;
}

void testOLED() {

	SSD1306::OLED *outLED = new SSD1306::OLED();

	outLED->write(0,0, std::string("World"));
	outLED->write(0,8, std::string("Hello"));
	outLED->write(0,16, std::string("Line 3"));
	outLED->write(0,24, std::string("Line 4"));

	int X = outLED->fx->getWidth();
	int Y = outLED->fx->getHeight();

	int max = (X > Y) ? X : Y;

	outLED->fx->clearScreen();
	outLED->fx->setTextColor(SSD1306::RGB::black, SSD1306::RGB::white);
	outLED->fx->setTextSize(1);

	outLED->fx->setCursor(0,0);
	outLED->fx->write("Hello\nWorld!");

	sleep(2);
	outLED->fx->clearScreen();
	outLED->fx->drawLine(0,0,X,Y, SSD1306::RGB::black);
	outLED->fx->drawLine(0,Y,X,0, SSD1306::RGB::black);
	for (int i = 0; i < max/5 -1; i += 10) {
		outLED->fx->drawRoundRect(i, i,(X - 2 * i),(Y - 2 * i),8, SSD1306::RGB::black);
	}
	outLED->fx->drawCircle(X/2,Y/2,max/4 - 1, SSD1306::RGB::black);
	outLED->fx->refreshScreen();

	sleep(2);

	outLED->fx->clearScreen();

	outLED->run();
	WAIT_THREAD_FINISH(outLED);
}

int main(int argc, char const *argv[]) {
	std::vector<std::string> args(argv +1, argv + argc);

	std::cout << "Checking overlays are loaded... \n" << std::flush;

	if (checkOverlays()) {
		cout << "OK" << std::endl;
	} else {
		return 0;
	}

//	testOLED();
//	return 0;

	if (args.size() == 4) {
		controller(atof(args[0].c_str()), atof(args[1].c_str()), atof(args[2].c_str()), atoi(args[3].c_str()));
	} else {
		controller(1,0,0,0);
	}

	return 0;
}
