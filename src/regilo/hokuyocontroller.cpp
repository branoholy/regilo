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

#include "regilo/hokuyocontroller.hpp"

#include <cmath>

#include <boost/algorithm/string/trim.hpp>

namespace regilo {

std::string HokuyoController::CMD_GET_VERSION = "V";
std::string HokuyoController::CMD_GET_SCAN = "G%03d%03d%02d";

HokuyoController::HokuyoController() : BaseController()
{
	init();
}

HokuyoController::HokuyoController(const std::string& logPath) : BaseController(logPath)
{
	init();
}

HokuyoController::HokuyoController(std::iostream& logStream) : BaseController(logStream)
{
	init();
}

void HokuyoController::init()
{
	RESPONSE_END = "\n\n";
	validFromStep = 44;
	validToStep = 725;
	maxStep = 768;
	startAngle = -135 * M_PI / 180;

	setScanParameters(0, maxStep, 1);
}

void HokuyoController::connect(const std::string& endpoint)
{
	this->endpoint = endpoint;

	stream.open(endpoint);

	ba::serial_port_base::baud_rate baudRate(115200);
	ba::serial_port_base::character_size charSize(8);

	stream.set_option(baudRate);
	stream.set_option(charSize);
}

std::map<std::string, std::string> HokuyoController::getVersionInfo()
{
	std::map<std::string, std::string> versionInfo;

	char status;
	sendCommand(CMD_GET_VERSION);
	deviceOutput >> status;;

	if(status == '0')
	{
		std::string line;
		while(std::getline(deviceOutput, line))
		{
			if(line.empty()) continue;

			std::size_t colonPos = line.find(':');
			std::string name = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);

			boost::algorithm::trim(name);
			boost::algorithm::trim(value);

			versionInfo[name] = value;
		}
	}

	return versionInfo;
}

void HokuyoController::setScanParameters(std::size_t fromStep, std::size_t toStep, std::size_t clusterCount)
{
	if(fromStep > maxStep) throw new std::invalid_argument("Invalid fromStep argument.");
	if(toStep > maxStep) throw new std::invalid_argument("Invalid fromStep argument.");
	if(clusterCount > 99) throw new std::invalid_argument("Invalid clusterCount argument.");
	if(fromStep > toStep) throw new std::invalid_argument("fromStep has to be lower than toStep.");

	this->fromStep = fromStep;
	this->toStep = toStep;
	this->clusterCount = clusterCount;
}

bool HokuyoController::parseScanData(std::istream& in, ScanData& data)
{
	char status;
	in >> status;
	if(status != '0') return false;

	double resolution = M_PI / 512;

	int lastId = 0;
	std::size_t step = fromStep - 1;
	while(in)
	{
		step++;

		char high, low;
		in >> high >> low;

		if(step < validFromStep || step > validToStep) continue;

		int id = lastId++;
		double angle = step * resolution + startAngle;
		int distance = ((high - '0') << 6) | (low - '0');
		int errorCode = 0;
		bool error = false;

		if(distance < 20)
		{
			errorCode = distance;
			distance = -1;
			error = true;
		}

		data.emplace_back(id, angle, distance, -1, errorCode, error);
	}

	return true;
}

}
