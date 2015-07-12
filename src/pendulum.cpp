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
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/String.h>
#include <rlutil.h>
#include <sys/stat.h>
#include <SSD1306/gfx.h>
#include <SSD1306/rgb_driver.h>
#include <SSD1306/ssd1306.h>
#include <threadedEQEP.h>
#include <unistd.h>
#include <cstdbool>
#include <iomanip>
#include <list>
#include <sstream>
#include <utility>
#include <vector>

using namespace std;

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Format.h"
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <iostream>
#include <string>

using Poco::Net::ServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Timestamp;
using Poco::ThreadPool;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using namespace Poco::JSON;

//@TODO Get rid of nasty global variables. Use Poco::Application maybe?
// Create new EQEPs object to monitor the pendulum & motor position
threadedEQEP *pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, ENCODER_PPR);
threadedEQEP *motorEQEP = new threadedEQEP(MOTOR_EQEP, MOTOR_PPR);
pid *Controller;

string GetValue(Object::Ptr aoJsonObject, const char *aszKey) {
    Poco::Dynamic::Var loVariable;
    string lsReturn;
    string lsKey(aszKey);

    // Get the member Variable
    //
    loVariable = aoJsonObject->get(lsKey);

    // Get the Value from the Variable
    //
    lsReturn = loVariable.convert<std::string>();

    return lsReturn;
}


class PageRequestHandler: public HTTPRequestHandler
	/// Return a HTML document with some JavaScript creating
	/// a WebSocket connection.
{
public:
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		response.setChunkedTransferEncoding(true);
		response.setContentType("text/html");
		std::ostream& ostr = response.send();
		ostr << "<html>";
		ostr << "<head>";
		ostr << "<title>WebSocketServer</title>";
		ostr << "<script type=\"text/javascript\">";
		ostr << "function WebSocketTest()";
		ostr << "{";
		ostr << "  if (\"WebSocket\" in window)";
		ostr << "  {";
		ostr << "    var ws = new WebSocket(\"ws://" << request.serverAddress().toString() << "/ws\");";
		ostr << "    ws.onopen = function()";
		ostr << "      {";
		ostr << "        ws.send(\"Hello, world!\");";
		ostr << "      };";
		ostr << "    ws.onmessage = function(evt)";
		ostr << "      { ";
		ostr << "        var msg = evt.data;";
		ostr << "        alert(\"Message received: \" + msg);";
		ostr << "        ws.close();";
		ostr << "      };";
		ostr << "    ws.onclose = function()";
		ostr << "      { ";
		ostr << "        alert(\"WebSocket closed.\");";
		ostr << "      };";
		ostr << "  }";
		ostr << "  else";
		ostr << "  {";
		ostr << "     alert(\"This browser does not support WebSockets.\");";
		ostr << "  }";
		ostr << "}";
		ostr << "</script>";
		ostr << "</head>";
		ostr << "<body>";
		ostr << "  <h1>WebSocket Server</h1>";
		ostr << "  <p><a href=\"javascript:WebSocketTest()\">Run WebSocket Script</a></p>";
		ostr << "</body>";
		ostr << "</html>";
	}
};


class WebSocketRequestHandler: public HTTPRequestHandler
	/// Handle a WebSocket connection.
{
public:
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		try
		{
			WebSocket ws(request, response);
			cout << "WebSocket connection established." << endl;
			char buffer[1024];
			int flags;
			int n;
			Parser loParser;
			Poco::Dynamic::Var parsedJSON;
			Poco::Dynamic::Var parsedJSONResult;
			Object::Ptr JSONObject;
			do
			{
				n = ws.receiveFrame(buffer, sizeof(buffer), flags);

				std::string rxJSON(buffer, n); // convert received frame into a string
				Poco::JSON::Parser Parser;
				Poco::Dynamic::Var result = Parser.parse(rxJSON); // parse JSON
				Poco::JSON::Object::Ptr obj = result.extract<Poco::JSON::Object::Ptr>();
				std::string action;
				std::string param;
				std::string out;
				std::ostringstream ossRx;
				std::ostringstream ossTx;
				obj->stringify(ossRx, 0);
				if (obj->has("action")) {
					action = obj->getValue<std::string>("action");
					param = "";
					if (obj->has("param")) {
						param = obj->getValue<std::string>("param");
					}
					if ( action == "get") {
						if (param == "pendulum") {
							obj->set("value", pendulumEQEP->getAngleDeg());
						} else
						if (param == "kp") {
							obj->set("value", Controller->getKP());
						} else
						if (param == "ki") {
							obj->set("value", Controller->getKI());
						} else
						if (param == "kd") {
							obj->set("value", Controller->getKD());
						}
					} else if (action == "set") {
						float val;
						if (obj->has("value")) {
							val = obj->getValue<float>("value");
						} else {
							val = -1;
						}
						if (param == "kp") {
							Controller->setKP(val);
							obj->set("value", val);
						} else
						if (param == "ki") {
							Controller->setKI(val);
							obj->set("value", val);
						} else
						if (param == "kd") {
							Controller->setKD(val);
							obj->set("value", val);
						}
					}
				}
				obj->remove("action");
				obj->stringify(ossTx,0);
				out = ossTx.str();
				ws.sendFrame(out.data(), out.size(), flags);
			}
			while (n > 0 || (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
			cout << "WebSocket connection closed." << endl;
		}
		catch (WebSocketException& exc)
		{
			switch (exc.code())
			{
			case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
				response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
				// fallthrough
			case WebSocket::WS_ERR_NO_HANDSHAKE:
			case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
			case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
				response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
				response.setContentLength(0);
				response.send();
				break;
			}
		}
	}
};


class RequestHandlerFactory: public HTTPRequestHandlerFactory
{
public:
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
		cout << "Request from "
			+ request.clientAddress().toString()
			+ ": "
			+ request.getMethod()
			+ " "
			+ request.getURI()
			+ " "
			+ request.getVersion() << endl;

		for (HTTPServerRequest::ConstIterator it = request.begin(); it != request.end(); ++it)
		{
			cout << it->first + ": " + it->second << endl;
		}

		if(request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
			return new WebSocketRequestHandler;
		else
			return new PageRequestHandler;
	}
};

>>>>>>> a4ce6b0 Remote get/set of PID parameters and sending pendulum angle working.
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
	fx.write("\n Raise the pendulum\n\n P:\n M:");
	fx.drawRoundRect(0,0,fx.getWidth(),fx.getHeight(), 8, SSD1306::RGB::black);
	// Start the thread running
	pendulumEQEP->run();

	// Wait until the pendulum is @ 180 +-1 deg
	// Assumes pendulum starts hanging vertically down
	while (abs(pendulumEQEP->getAngleDeg()) < 179 || abs(pendulumEQEP->getAngleDeg() > 181))  {
		fx.setCursor(18, 24);
		fx.write(to_string(pendulumEQEP->getAngleDeg()).c_str());
		fx.refreshScreen();
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
		fx.setCursor(18,24);
		fx.write(to_string(pendulumEQEP->getAngleDeg()).c_str());
		fx.setCursor(18,32);
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

	std::cout << "Checking overlays" << std::endl;
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

//	testOLED();
//	motorTest();
	controller();
	return 0;
}
