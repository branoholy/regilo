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

#ifndef REGILO_NEATOCONTROLLER_HPP
#define REGILO_NEATOCONTROLLER_HPP

#include <cmath>

#include <boost/algorithm/string.hpp>

#include "scancontroller.hpp"
#include "serialcontroller.hpp"
#include "socketcontroller.hpp"

namespace regilo {

/**
 * @brief The INeatoController interface is used for the NeatoController class.
 */
class INeatoController
{
public:
	/**
	 * @brief Default destructor.
	 */
	virtual ~INeatoController() = default;

	/**
	 * @brief Get whether the Neato is in the test mode.
	 * @return true/false
	 */
	virtual bool getTestMode() const = 0;

	/**
	 * @brief Set or unset the test mode.
	 * @param testMode true/false
	 */
	virtual void setTestMode(bool testMode) = 0;

	/**
	 * @brief Get whether the Neato has LDS rotation on or off.
	 * @return true/false
	 */
	virtual bool getLdsRotation() const = 0;

	/**
	 * @brief Set LDS rotation on or off.
	 * @param ldsRotation true/false
	 */
	virtual void setLdsRotation(bool ldsRotation) = 0;

	/**
	 * @brief Set the specified motor to run in a direction at a requested speed.
	 * @param left Distance in millimeters to drive the left wheel (pos = forward, neg = backward).
	 * @param right Distance in millimeters to drive the right wheel (pos = forward, neg = backward).
	 * @param speed Speed in millimeters/second.
	 */
	virtual void setMotor(int left, int right, int speed) = 0;

	/**
	 * @brief Get the current scheduler time.
	 * @return "DayOfWeek HourOf24:Min:Sec" (example: "Sunday 13:57:09")
	 */
	virtual std::string getTime() = 0;
};

/**
 * @brief The NeatoController class is used to communicate with the Neato robot.
 */
template<typename ProtocolController>
class NeatoController : public INeatoController, public ScanController<ProtocolController>
{
private:
	bool testMode = false;
	bool ldsRotation = false;

protected:
	virtual inline std::string getScanCommand() const override { return CMD_GET_LDS_SCAN; }
	virtual bool parseScanData(std::istream& in, ScanData& data) override;

public:
	static std::string ON; ///< A string that represents the ON value.
	static std::string OFF; ///< A string that represents the OFF value.
	static std::string LDS_SCAN_HEADER; ///< A header of the LDS scan output.
	static std::string LDS_SCAN_FOOTER; ///< A footer of the LDS scan output.

	static std::string CMD_TEST_MODE; ///< A template for the `testmode` command.
	static std::string CMD_SET_LDS_ROTATION; ///< A template for the `setldsrotation` command.
	static std::string CMD_SET_MOTOR; ///< A template for the `setmotor` command.
	static std::string CMD_GET_TIME; ///< A template for the `gettime` command.
	static std::string CMD_GET_LDS_SCAN; ///< A template for the `getldsscan` command.

	/**
	 * @brief Default constructor.
	 */
	NeatoController();

	/**
	 * @brief Constructor with a log file specified by a path.
	 * @param logPath Path to the log file.
	 */
	NeatoController(const std::string& logPath);

	/**
	 * @brief Constructor with a log specified by a stream.
	 * @param logStream The log stream.
	 */
	NeatoController(std::iostream& logStream);

	/**
	 * @brief Default destructor.
	 */
	virtual ~NeatoController() = default;

	virtual inline bool getTestMode() const override { return testMode; }

	virtual void setTestMode(bool testMode) override;

	virtual inline bool getLdsRotation() const override { return ldsRotation; }

	virtual void setLdsRotation(bool ldsRotation) override;

	virtual void setMotor(int left, int right, int speed) override;

	virtual std::string getTime() override;
};

extern template class NeatoController<SerialController>;
extern template class NeatoController<SocketController>;

typedef NeatoController<SerialController> NeatoSerialController;
typedef NeatoController<SocketController> NeatoSocketController;

template<typename ProtocolController>
std::string NeatoController<ProtocolController>::ON = "on";

template<typename ProtocolController>
std::string NeatoController<ProtocolController>::OFF = "off";

template<typename ProtocolController>
std::string NeatoController<ProtocolController>::LDS_SCAN_HEADER = "AngleInDegrees,DistInMM,Intensity,ErrorCodeHEX";

template<typename ProtocolController>
std::string NeatoController<ProtocolController>::LDS_SCAN_FOOTER = "ROTATION_SPEED,";

template<typename ProtocolController>
std::string NeatoController<ProtocolController>::CMD_TEST_MODE = "testmode %s";

template<typename ProtocolController>
std::string NeatoController<ProtocolController>::CMD_SET_LDS_ROTATION = "setldsrotation %s";

template<typename ProtocolController>
std::string NeatoController<ProtocolController>::CMD_SET_MOTOR = "setmotor %d %d %d";

template<typename ProtocolController>
std::string NeatoController<ProtocolController>::CMD_GET_TIME = "gettime";

template<typename ProtocolController>
std::string NeatoController<ProtocolController>::CMD_GET_LDS_SCAN = "getldsscan";

template<typename ProtocolController>
NeatoController<ProtocolController>::NeatoController() : ScanController<ProtocolController>()
{
	this->RESPONSE_END = std::string(1, 0x1a);
}

template<typename ProtocolController>
NeatoController<ProtocolController>::NeatoController(const std::string& logPath) : ScanController<ProtocolController>(logPath)
{
	this->RESPONSE_END = std::string(1, 0x1a);
}

template<typename ProtocolController>
NeatoController<ProtocolController>::NeatoController(std::iostream& logStream) : ScanController<ProtocolController>(logStream)
{
	this->RESPONSE_END = std::string(1, 0x1a);
}

template<typename ProtocolController>
void NeatoController<ProtocolController>::setTestMode(bool testMode)
{
	this->sendFormattedCommand(CMD_TEST_MODE, (testMode ? ON : OFF).c_str());
	this->testMode = testMode;
}

template<typename ProtocolController>
void NeatoController<ProtocolController>::setLdsRotation(bool ldsRotation)
{
	this->sendFormattedCommand(CMD_SET_LDS_ROTATION, (ldsRotation ? ON : OFF).c_str());
	this->ldsRotation = ldsRotation;
}

template<typename ProtocolController>
void NeatoController<ProtocolController>::setMotor(int left, int right, int speed)
{
	this->sendFormattedCommand(CMD_SET_MOTOR, left, right, speed);
}

template<typename ProtocolController>
bool NeatoController<ProtocolController>::parseScanData(std::istream& in, ScanData& data)
{
	int lastId = 0;
	double M_PI_180 = M_PI / 180.0;

	std::string line;
	std::getline(in, line);
	boost::algorithm::trim(line);

	if(line == NeatoController<ProtocolController>::LDS_SCAN_HEADER)
	{
		while(true)
		{
			std::getline(in, line);
			boost::algorithm::trim(line);

			if(boost::algorithm::starts_with(line, NeatoController<ProtocolController>::LDS_SCAN_FOOTER))
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

template<typename ProtocolController>
std::string NeatoController<ProtocolController>::getTime()
{
	return this->sendCommand(CMD_GET_TIME);
}

}

#endif // REGILO_NEATOCONTROLLER_HPP
