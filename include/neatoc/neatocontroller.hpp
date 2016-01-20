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

#ifndef NEATOC_NEATOCONTROLLER_HPP
#define NEATOC_NEATOCONTROLLER_HPP

#include <boost/asio/ip/tcp.hpp>

#include "controller.hpp"

namespace neatoc {

namespace bai = boost::asio::ip;

/**
 * @brief The NeatoController class is used to communicate with the Neato robot.
 */
class NeatoController : public BaseController<bai::tcp::socket>
{
private:
	bool testMode;
	bool ldsRotation;

protected:
	void init();

	inline std::string getScanCommand() const override { return CMD_GET_LDS_SCAN; }
	bool parseScanData(std::istream& in, ScanData& data);

public:
	static std::string ON;
	static std::string OFF;
	static std::string LDS_SCAN_HEADER;
	static std::string LDS_SCAN_FOOTER;

	static std::string CMD_TEST_MODE;
	static std::string CMD_SET_LDS_ROTATION;
	static std::string CMD_SET_MOTOR;
	static std::string CMD_GET_TIME;
	static std::string CMD_GET_LDS_SCAN;

	/**
	 * @brief Default constructor.
	 */
	NeatoController();

	/**
	 * @brief Controller with a log file specified by a path.
	 * @param logPath Path to the log file.
	 */
	NeatoController(const std::string& logPath);

	/**
	 * @brief Controller with a log specified by a stream.
	 * @param logStream The log stream.
	 */
	NeatoController(std::iostream& logStream);

	/**
	 * @brief Default destructor.
	 */
	virtual ~NeatoController();

	/**
	 * @brief Connect the controller to the Neato robot.
	 * @param endpoint The endpoint with the IP address and port of the Neato robot (e.g. "10.0.0.1:12345").
	 */
	void connect(const std::string& endpoint);

	/**
	 * @brief Connect the controller to the Neato robot.
	 * @param ip The IP address of the Neato robot (e.g. "10.0.0.1")
	 * @param port The port number of the Neato robot (e.g. 12345)
	 */
	void connect(const std::string& ip, unsigned short port);

	/**
	 * @brief Connect the controller to the Neato robot.
	 * @param endpoint The endpoint of the Neato robot.
	 */
	void connect(const bai::tcp::endpoint& endpoint);

	/**
	 * @brief Get the endpoint of the connected Neato robot.
	 * @return The endpoint.
	 */
	std::string getEndpoint() const;

	/**
	 * @brief Get whether the Neato is in the test mode.
	 * @return true/false
	 */
	inline bool getTestMode() const { return testMode; }

	/**
	 * @brief Set or unset the test mode.
	 * @param testMode true/false
	 */
	void setTestMode(bool testMode);

	/**
	 * @brief Get whether the Neato has LDS rotation on or off.
	 * @return true/false
	 */
	inline bool getLdsRotation() const { return ldsRotation; }

	/**
	 * @brief Set LDS rotation on or off.
	 * @param ldsRotation true/false
	 */
	void setLdsRotation(bool ldsRotation);

	/**
	 * @brief Set the specified motor to run in a direction at a requested speed.
	 * @param left Distance in millimeters to drive the left wheel (pos = forward, neg = backward).
	 * @param right Distance in millimeters to drive the right wheel (pos = forward, neg = backward).
	 * @param speed Speed in millimeters/second.
	 */
	void setMotor(int left, int right, int speed);

	/**
	 * @brief Get the current scheduler time.
	 * @return "DayOfWeek HourOf24:Min:Sec" (example: "Sunday 13:57:09")
	 */
	std::string getTime();
};

}

#endif // NEATOC_NEATOCONTROLLER_HPP
