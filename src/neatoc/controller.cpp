/*
 * NeatoC
 * Copyright (C) 2015-2016  Branislav Holý <branoholy@gmail.com>
 *
 * This file is part of NeatoC.
 *
 * NeatoC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NeatoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NeatoC.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "neatoc/controller.hpp"

#include <cmath>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>

#include <chrono>

#include "neatoc/log.hpp"

namespace neatoc {

namespace bai = boost::asio::ip;

std::string Controller::ON = "on";
std::string Controller::OFF = "off";
std::string Controller::LDS_SCAN_HEADER = "AngleInDegrees,DistInMM,Intensity,ErrorCodeHEX";
std::string Controller::LDS_SCAN_FOOTER = "ROTATION_SPEED,";

std::string Controller::TEST_MODE = "testmode %s";
std::string Controller::SET_LDS_ROTATION = "setldsrotation %s";
std::string Controller::SET_MOTOR = "setmotor %d %d %d";
std::string Controller::GET_TIME = "gettime";
std::string Controller::GET_LDS_SCAN = "getldsscan";

char Controller::REQUEST_END = '\n';
char Controller::RESPONSE_END = 0x1a;

Controller::Controller() :
	lastScanId(0),
	testMode(false),
	ldsRotation(false),
	socket(ioService),
	socketIStream(&socketIStreamBuffer),
	socketOStream(&socketOStreamBuffer),
	log(nullptr), logFile(nullptr)
{
}

Controller::Controller(const std::string& logPath) : Controller()
{
	if(logPath.length() > 0)
	{
		logFile = new std::fstream(logPath, std::fstream::in | std::fstream::out | std::fstream::app);
		log = new Log(*logFile);
		this->logPath = logPath;
	}
}

Controller::Controller(std::iostream& logStream) : Controller()
{
	this->log = new Log(logStream);
}

Controller::~Controller()
{
	if(log != nullptr) delete log;
	if(logFile != nullptr) delete logFile;

	if(socket.is_open())
	{
		socket.shutdown(bai::tcp::socket::shutdown_both);
		socket.close();
	}
}

void Controller::connect(const std::string& endpoint)
{
	std::string ip = endpoint;
	unsigned short port = 0;

	std::size_t colonPos = endpoint.find(':');
	if(colonPos != std::string::npos)
	{
		ip = endpoint.substr(0, colonPos);
		port = stoul(endpoint.substr(colonPos + 1));
	}

	connect(ip, port);
}

void Controller::connect(const std::string& ip, unsigned short port)
{
	connect(bai::tcp::endpoint(bai::address::from_string(ip), port));
}

void Controller::connect(const bai::tcp::endpoint& endpoint)
{
	socket.connect(endpoint);
}

void Controller::setTestMode(bool testMode)
{
	createAndSendCommand(TEST_MODE, (testMode ? ON : OFF).c_str());
	this->testMode = testMode;
}

void Controller::setLdsRotation(bool ldsRotation)
{
	createAndSendCommand(SET_LDS_ROTATION, (ldsRotation ? ON : OFF).c_str());
	this->ldsRotation = ldsRotation;
}

void Controller::setMotor(int left, int right, int speed)
{
	createAndSendCommand(SET_MOTOR, left, right, speed);
}

ScanData Controller::getLdsScan(bool fromScanner)
{
	ScanData data;

	if(fromScanner)
	{
		sendCommand(GET_LDS_SCAN);
		neatoOutput >> data;
	}
	else
	{
		std::istringstream response(log->readCommand(GET_LDS_SCAN));
		response >> data;
	}
	if(!data.empty()) data.scanId = lastScanId++;

	return data;
}

std::string Controller::getTime()
{
	return sendCommand(GET_TIME);
}

std::string Controller::sendCommand(const std::string& command)
{
	neatoInput << command;
	return sendCommand();
}

std::string Controller::sendCommand()
{
	if(!socket.is_open()) throw NetworkException("Socket is not connected.");

	neatoInput << REQUEST_END;

	std::string input = neatoInput.str();
	socketOStream << input;
	boost::asio::write(socket, socketOStreamBuffer);
	neatoInput.clear();
	neatoInput.str("");

	boost::asio::read_until(socket, socketIStreamBuffer, RESPONSE_END);

	std::string cmdInput;
	std::getline(socketIStream, cmdInput, REQUEST_END);
	cmdInput.pop_back();

	std::string output;
	std::getline(socketIStream, output, RESPONSE_END);
	neatoOutput.clear();
	neatoOutput.str(output);

	if(log != nullptr) log->write(input, output);

	return output;
}

NetworkException::NetworkException(const std::string& message) :
	runtime_error(message)
{
}

}
