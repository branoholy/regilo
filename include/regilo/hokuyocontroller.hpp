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

#ifndef REGILO_HOKUYOCONTROLLER_HPP
#define REGILO_HOKUYOCONTROLLER_HPP

#include <cmath>
#include <map>

#include <boost/algorithm/string/trim.hpp>

#include "scancontroller.hpp"
#include "serialcontroller.hpp"
#include "socketcontroller.hpp"

namespace regilo {

/**
 * @brief The IHokuyoController interface is used for the HokuyoController class.
 */
class IHokuyoController
{
public:
	/**
	 * @brief Default destructor.
	 */
	virtual ~IHokuyoController() = default;

	/**
	 * @brief Return information about the scanner version.
	 * @return Key-value pairs with the information.
	 */
	virtual std::map<std::string, std::string> getVersionInfo() = 0;
};

/**
 * @brief The HokuyoController class is used to communicate with the Hokuyo scanner.
 */
template<typename ProtocolController>
class HokuyoController : public IHokuyoController, public ScanController<ProtocolController>
{
private:
	std::size_t validFromStep = 44;
	std::size_t validToStep = 725;
	std::size_t maxStep = 768;
	std::size_t fromStep = 0;
	std::size_t toStep = maxStep;
	std::size_t clusterCount = 1;
	double startAngle = -135 * M_PI / 180;

protected:
	virtual inline std::string getScanCommand() const override { return this->createFormattedCommand(CMD_GET_SCAN, fromStep, toStep, clusterCount); }
	virtual bool parseScanData(std::istream& in, ScanData& data) override;

public:
	static std::string CMD_GET_VERSION; ///< A command for getting the scanner version.
	static std::string CMD_GET_SCAN; ///< A command for getting a scan.

	/**
	 * @brief Default constructor.
	 */
	HokuyoController();

	/**
	 * @brief Constructor with a log file specified by a path.
	 * @param logPath Path to the log file.
	 */
	HokuyoController(const std::string& logPath);

	/**
	 * @brief Constructor with a log specified by a stream.
	 * @param logStream The log stream.
	 */
	HokuyoController(std::iostream& logStream);

	/**
	 * @brief Default destructor.
	 */
	virtual ~HokuyoController() = default;

	virtual std::map<std::string, std::string> getVersionInfo() override;

	/**
	 * @brief Set parameters for the scan command.
	 * @param fromStep The starting step [0; maxStep].
	 * @param toStep The ending steop [0; maxStep].
	 * @param clusterCount The cluster count [0; 99].
	 */
	void setScanParameters(std::size_t fromStep, std::size_t toStep, std::size_t clusterCount);
};

extern template class HokuyoController<SerialController>;
extern template class HokuyoController<SocketController>;

typedef HokuyoController<SerialController> HokuyoSerialController;
typedef HokuyoController<SocketController> HokuyoSocketController;

template<typename ProtocolController>
std::string HokuyoController<ProtocolController>::CMD_GET_VERSION = "V";

template<typename ProtocolController>
std::string HokuyoController<ProtocolController>::CMD_GET_SCAN = "G%03d%03d%02d";

template<typename ProtocolController>
HokuyoController<ProtocolController>::HokuyoController() : ScanController<ProtocolController>()
{
	this->RESPONSE_END = "\n\n";
}

template<typename ProtocolController>
HokuyoController<ProtocolController>::HokuyoController(const std::string& logPath) : ScanController<ProtocolController>(logPath)
{
	this->RESPONSE_END = "\n\n";
}

template<typename ProtocolController>
HokuyoController<ProtocolController>::HokuyoController(std::iostream& logStream) : ScanController<ProtocolController>(logStream)
{
	this->RESPONSE_END = "\n\n";
}

template<typename ProtocolController>
std::map<std::string, std::string> HokuyoController<ProtocolController>::getVersionInfo()
{
	std::map<std::string, std::string> versionInfo;

	if(ProtocolController::template sendCommand<char>(CMD_GET_VERSION) == '0')
	{
		std::string line;
		while(std::getline(this->deviceOutput, line))
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

template<typename ProtocolController>
void HokuyoController<ProtocolController>::setScanParameters(std::size_t fromStep, std::size_t toStep, std::size_t clusterCount)
{
	if(fromStep > maxStep) throw std::invalid_argument("Invalid fromStep argument.");
	if(toStep > maxStep) throw std::invalid_argument("Invalid fromStep argument.");
	if(clusterCount > 99) throw std::invalid_argument("Invalid clusterCount argument.");
	if(fromStep > toStep) throw std::invalid_argument("fromStep has to be lower than toStep.");

	this->fromStep = fromStep;
	this->toStep = toStep;
	this->clusterCount = clusterCount;
}

template<typename ProtocolController>
bool HokuyoController<ProtocolController>::parseScanData(std::istream& in, ScanData& data)
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

#endif // REGILO_HOKUYOCONTROLLER_HPP
