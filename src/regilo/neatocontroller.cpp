/*
 * Regilo
 * Copyright (C) 2015-2016  Branislav Holý <branoholy@gmail.com>
 *
 * This file is part of Regilo.
 *
 * Regilo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Regilo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Regilo.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "regilo/neatocontroller.hpp"

#include <cmath>

#include <boost/algorithm/string.hpp>

namespace regilo {

namespace bai = boost::asio::ip;

std::string NeatoController::ON = "on";
std::string NeatoController::OFF = "off";
std::string NeatoController::LDS_SCAN_HEADER = "AngleInDegrees,DistInMM,Intensity,ErrorCodeHEX";
std::string NeatoController::LDS_SCAN_FOOTER = "ROTATION_SPEED,";

std::string NeatoController::CMD_TEST_MODE = "testmode %s";
std::string NeatoController::CMD_SET_LDS_ROTATION = "setldsrotation %s";
std::string NeatoController::CMD_SET_MOTOR = "setmotor %d %d %d";
std::string NeatoController::CMD_GET_TIME = "gettime";
std::string NeatoController::CMD_GET_LDS_SCAN = "getldsscan";

NeatoController::NeatoController() : BaseController()
{
	init();
}

NeatoController::NeatoController(const std::string& logPath) : BaseController(logPath)
{
	init();
}

NeatoController::NeatoController(std::iostream& logStream) : BaseController(logStream)
{
	init();
}

NeatoController::~NeatoController()
{
	if(stream.is_open())
	{
		stream.shutdown(bai::tcp::socket::shutdown_both);
	}
}

void NeatoController::init()
{
	testMode = false;
	ldsRotation = false;

	RESPONSE_END.clear();
	RESPONSE_END.push_back(0x1a);
}

void NeatoController::connect(const std::string& endpoint)
{
	std::string ip = endpoint;
	unsigned short port = 0;

	std::size_t colonPos = endpoint.find(':');
	if(colonPos != std::string::npos)
	{
		ip = endpoint.substr(0, colonPos);
		port = std::stoul(endpoint.substr(colonPos + 1));
	}

	connect(ip, port);
}

void NeatoController::connect(const std::string& ip, unsigned short port)
{
	connect(bai::tcp::endpoint(bai::address::from_string(ip), port));
}

void NeatoController::connect(const bai::tcp::endpoint& endpoint)
{
	stream.connect(endpoint);
}

std::string NeatoController::getEndpoint() const
{
	StreamType::endpoint_type endpoint = stream.remote_endpoint();

	std::string ip = endpoint.address().to_string();
	std::string port = std::to_string(endpoint.port());

	return ip + ':' + port;
}

void NeatoController::setTestMode(bool testMode)
{
	createCommandAndSend(CMD_TEST_MODE, (testMode ? ON : OFF).c_str());
	this->testMode = testMode;
}

void NeatoController::setLdsRotation(bool ldsRotation)
{
	createCommandAndSend(CMD_SET_LDS_ROTATION, (ldsRotation ? ON : OFF).c_str());
	this->ldsRotation = ldsRotation;
}

void NeatoController::setMotor(int left, int right, int speed)
{
	createCommandAndSend(CMD_SET_MOTOR, left, right, speed);
}

bool NeatoController::parseScanData(std::istream& in, ScanData& data)
{
	int lastId = 0;
	double M_PI_180 = M_PI / 180.0;

	std::string line;
	std::getline(in, line);
	boost::algorithm::trim(line);

	if(line == NeatoController::LDS_SCAN_HEADER)
	{
		while(true)
		{
			std::getline(in, line);
			boost::algorithm::trim(line);

			if(boost::algorithm::starts_with(line, NeatoController::LDS_SCAN_FOOTER))
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
				bool error = (errorCode != 0);

				if(error) distance = -1;

				data.emplace_back(id, angle, distance, intensity, errorCode, error);
			}
		}

		return true;
	}

	return false;
}

std::string NeatoController::getTime()
{
	return sendCommand(CMD_GET_TIME);
}

}
