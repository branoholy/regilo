/*
 * NeatoC
 * Copyright (C) 2015  Branislav Holý <branoholy@gmail.com>
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

#include <boost/algorithm/string.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read_until.hpp>

namespace bai = boost::asio::ip;

const std::string neatoc::Controller::ON = "on";
const std::string neatoc::Controller::OFF = "off";
const std::string neatoc::Controller::LDS_SCAN_HEADER = "AngleInDegrees,DistInMM,Intensity,ErrorCodeHEX";
const std::string neatoc::Controller::LDS_SCAN_FOOTER = "ROTATION_SPEED,";

const std::string neatoc::Controller::TEST_MODE = "testmode ";
const std::string neatoc::Controller::SET_LDS_ROTATION = "setldsrotation ";
const std::string neatoc::Controller::SET_MOTOR = "setmotor ";
const std::string neatoc::Controller::GET_TIME = "gettime";
const std::string neatoc::Controller::GET_LDS_SCAN = "getldsscan";

const char neatoc::Controller::REQUEST_END = '\n';
const char neatoc::Controller::RESPONSE_END = 0x1a;

neatoc::Controller::Controller() :
	lastScanId(0),
	testMode(false),
	ldsRotation(false),
	socket(ioService),
	socketIStream(&socketIStreamBuffer),
	socketOStream(&socketOStreamBuffer)
{
}

neatoc::Controller::~Controller()
{
	if(socket.is_open())
	{
		socket.shutdown(bai::tcp::socket::shutdown_both);
		socket.close();
	}
}

void neatoc::Controller::connect(const std::string& endpoint)
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

void neatoc::Controller::connect(const std::string& ip, unsigned short port)
{
	connect(bai::tcp::endpoint(bai::address::from_string(ip), port));
}

void neatoc::Controller::connect(const bai::tcp::endpoint& endpoint)
{
	socket.connect(endpoint);
}

void neatoc::Controller::setTestMode(bool testMode)
{
	neatoInput << TEST_MODE << (testMode ? ON : OFF);
	sendCommand();

	this->testMode = testMode;
}

void neatoc::Controller::setLdsRotation(bool ldsRotation)
{
	neatoInput << SET_LDS_ROTATION << (ldsRotation ? ON : OFF);
	sendCommand();

	this->ldsRotation = ldsRotation;
}

void neatoc::Controller::setMotor(int left, int right, int speed)
{
	neatoInput << SET_MOTOR << left << ' ' << right << ' ' << speed;
	sendCommand();
}

neatoc::ScanData neatoc::Controller::getLdsScan()
{
	neatoc::ScanData data;
	sendCommand(GET_LDS_SCAN);

	std::string line;
	std::getline(neatoOutput, line);

	if(line == LDS_SCAN_HEADER)
	{
		data.scanId = lastScanId++;

		int lastId = 0;
		double M_PI_180 = M_PI / 180.0;

		while(true)
		{
			std::getline(neatoOutput, line);

			if(boost::algorithm::starts_with(line, LDS_SCAN_FOOTER))
			{
				std::vector<std::string> values;
				boost::algorithm::split(values, line, boost::algorithm::is_any_of(","));
				data.rotationSpeed = std::stod(values.at(1));

				break;
			}
			else
			{
				std::vector<std::string> values;
				boost::algorithm::split(values, line, boost::algorithm::is_any_of(","));

				int id = lastId++;
				double angle = std::stod(values.at(0)) * M_PI_180;
				double distance = std::stod(values.at(1));
				int intensity = std::stoi(values.at(2));
				int errorCode = std::stoi(values.at(3));

				data.emplace_back(id, angle, distance, intensity, errorCode);
			}
		}
	}


	return data;
}

std::string neatoc::Controller::getTime()
{
	return sendCommand(GET_TIME);
}

std::string neatoc::Controller::sendCommand(const std::string& command)
{
	neatoInput << command;
	return sendCommand();
}

std::string neatoc::Controller::sendCommand()
{
	if(!socket.is_open()) throw neatoc::NetworkException("Socket is not connected.");

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

	return output;
}

neatoc::NetworkException::NetworkException(const std::string& message) :
	runtime_error(message)
{
}
