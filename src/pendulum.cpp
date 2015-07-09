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

			    string lsJson;
			    Parser loParser;
			    string out;
			    lsJson = string(buffer);

			    cout << lsJson << endl;

			    // Parse the JSON and get the Results
			    Poco::Dynamic::Var loParsedJson = loParser.parse(lsJson);
			    Poco::Dynamic::Var loParsedJsonResult = loParser.result();

			    // Get the JSON Object
			    Object::Ptr loJsonObject = loParsedJsonResult.extract<Object::Ptr>();
			    string action = GetValue(loJsonObject, "action");
			    if (action.find("get") != string::npos) {
			    	string param = GetValue(loJsonObject, "param");
			    	out = "{ \"param\": \"" + param + "\", \"value\": \"";
					if (param.find("pendulum") != string::npos) {
						out += std::to_string(pendulumEQEP->getAngleDeg()) + "\" }";
					} else if (param.find("kp") != string::npos) {
						out += std::to_string(Controller->getKP()) + "\" }";;
						cout << "kp: " << out << endl;
					} else if (param.find("ki") != string::npos) {
						out += std::to_string(Controller->getKI()) + "\" }";;
						cout << "ki: " << out << endl;
					} else if (param.find("kd") != string::npos) {
						out += std::to_string(Controller->getKD()) + "\" }";;
						cout << "kd: " << out << endl;
					}
					ws.sendFrame(out.data(), out.length()); // Don't include a trailing \0
				} else {
					ws.sendFrame(buffer, n, flags);
				}
				cout << "Frame received (length=" << std::to_string(n) << ", flags=" << std::hex << unsigned(flags) << ")." << endl;
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

void controller() {

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
	Controller = new pid(11.7, 10, 8, 40, pendulumEQEP, motorEQEP);

	Controller->run();

	// Let the thread run for a bit
	fx.setCursor(6,8);
	fx.write("PID Running ....  ");
	int count = 0;
	while (count < 500)  {
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

	if (!checkOverlays()) {
		rlutil::setColor(rlutil::WHITE);
		cout << "Are the overlays loaded?" << std::endl;
		return -1;
	}

	ServerSocket svs(9980);
	// set-up a HTTPServer instance
	HTTPServer srv(new RequestHandlerFactory, svs, new HTTPServerParams);
	// start the HTTPServer
	srv.start();
	controller();
	sleep(120);

	srv.stop();

	cout << "Done!" << endl;
	return 0;
}
