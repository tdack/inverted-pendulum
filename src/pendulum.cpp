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

#include <BlackLib/BlackI2C/BlackI2C.h>
#include <BlackLib/BlackThread/BlackThread.h>
#include <pendulum.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/String.h>
#include <unistd.h>
#include <list>
#include <sstream>
#include <utility>

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


class PageRequestHandler: public HTTPRequestHandler
	/// Return a HTML document
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
		ostr << "</head>";
		ostr << "<body>";
		ostr << "  <h1>WebSocket Server</h1>";
		ostr << "  <p>Connect client application using web socket protocol</p>";
		ostr << "</body>";
		ostr << "</html>";
	}
};

class WebSocketRequestHandler: public HTTPRequestHandler
	/// Handle a WebSocket connection.
{
public:
	WebSocketRequestHandler(threadedEQEP *_eqep, pid *_controller)
		: eqep(_eqep), controller(_controller)
	{}

	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		try
		{
			WebSocket ws(request, response);
			cout << "WebSocket connection established with " << request.clientAddress().toString() << endl;
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
							obj->set("value", eqep->getAngleDeg());
						} else
						if (param == "kp") {
							obj->set("value", controller->getKP());
						} else
						if (param == "ki") {
							obj->set("value", controller->getKI());
						} else
						if (param == "kd") {
							obj->set("value", controller->getKD());
						}
					} else if (action == "set") {
						float val;
						if (obj->has("value")) {
							val = obj->getValue<float>("value");
						} else {
							val = 0;
						}
						if (param == "kp") {
							controller->setKP(val);
							obj->set("value", val);
						} else
						if (param == "ki") {
							controller->setKI(val);
							obj->set("value", val);
						} else
						if (param == "kd") {
							controller->setKD(val);
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
private:
	threadedEQEP *eqep;
	pid* controller;
};


class RequestHandlerFactory: public HTTPRequestHandlerFactory
{
public:
	RequestHandlerFactory(threadedEQEP *_eqep, pid *_controller)
	: eqep(_eqep), controller(_controller)
	{}

	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
		if(request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
			return new WebSocketRequestHandler(eqep, controller);
		else
			return new PageRequestHandler;
	}
private:
	threadedEQEP *eqep;
	pid* controller;
};

void controller(double kp, double ki, double kd) {
	// Create new EQEPs object to monitor the pendulum & motor position
	threadedEQEP *pendulumEQEP = new threadedEQEP(PENDULUM_EQEP, PENDULUM_PPR);
	threadedEQEP *motorEQEP = new threadedEQEP(MOTOR_EQEP, MOTOR_PPR);

	// Create a new PID controller thread
	pid *Controller = new pid(11.7, kp, ki, kd, pendulumEQEP, motorEQEP);

	// set-up a HTTPServer instance
	ServerSocket svs(9980);
	HTTPServer srv(new RequestHandlerFactory(pendulumEQEP, Controller), svs, new HTTPServerParams);

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
	pendulumEQEP->setDeg(180.0);
	pendulumEQEP->run();

	// Wait until the pendulum is @ 0 +/-1 deg
	// Assumes pendulum starts hanging vertically down @ 180 deg
	double angle;
	do {
		angle = abs(pendulumEQEP->getAngleDeg());
		angle = angle >= 360 ? angle - 360 : angle;
		fx.setCursor(18, 24);
		fx.write(to_string(pendulumEQEP->getAngleDeg()).c_str());
		fx.refreshScreen();
	} while (angle < -1 || angle > 1);


	// Start the PID Controller thread running
	Controller->run();

	// Start the HTTP server
	srv.start();

	fx.setCursor(6,8);
	fx.write("PID Running ....  ");
	int count = 0;
	while (count < 500)  { 	// Let the thread run for a bit
		count++;
		angle = pendulumEQEP->getAngleDeg();
		if (abs(angle) > 25) {
			// Jump out of loop if pendulum goes too far from vertical
			std::cout << " Bailing out!!" << std::endl;
			break;
		}
		fx.setCursor(18,24);
		fx.write(to_string(angle).c_str());
		fx.setCursor(18,32);
		fx.write(to_string(motorEQEP->getAngleDeg()).c_str());
		fx.setCursor(110,54);
		fx.write(to_string(count).c_str());
		fx.refreshScreen();
	}

	srv.stopAll(true);

	Controller->stop();
	pendulumEQEP->stop();
	motorEQEP->stop();

	// Don't quit until all threads are finished
	WAIT_THREAD_FINISH(Controller);
	WAIT_THREAD_FINISH(pendulumEQEP);
	WAIT_THREAD_FINISH(motorEQEP);

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

int main(int argc, char const *argv[]) {

	std::vector<std::string> args(argv +1, argv + argc);

	if (!checkOverlays()) {
		rlutil::setColor(rlutil::WHITE);
		cout << "Loading Overlays ..." << std::endl;
		if (system("./load_overlays.sh") == -1) {
			return -1;
		}
	}

	if (args.size() == 3) {
		controller(atof(args[0].c_str()), atof(args[1].c_str()), atof(args[2].c_str()));
	} else {
		controller(0,0,0);
	}

	cout << "Done!" << endl;
	return 0;
}
