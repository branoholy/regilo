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

#ifndef NEATOC_CONTROLLER_HPP
#define NEATOC_CONTROLLER_HPP

#include <sstream>

#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "scandata.hpp"

namespace neatoc {

namespace bai = boost::asio::ip;

/**
 * @brief The Controller class is used to communicate with the Neato robot.
 */
class Controller
{
private:
	std::size_t lastScanId;

	bool testMode;
	bool ldsRotation;

	boost::asio::io_service ioService;
	bai::tcp::socket socket;

	boost::asio::streambuf socketIStreamBuffer;
	std::istream socketIStream;
	std::istringstream neatoOutput;

	boost::asio::streambuf socketOStreamBuffer;
	std::ostream socketOStream;
	std::ostringstream neatoInput;

	std::string sendCommand();

public:
	static const std::string ON;
	static const std::string OFF;
	static const std::string LDS_SCAN_HEADER;
	static const std::string LDS_SCAN_FOOTER;

	static const std::string TEST_MODE;
	static const std::string SET_LDS_ROTATION;
	static const std::string SET_MOTOR;
	static const std::string GET_TIME;
	static const std::string GET_LDS_SCAN;

	static const char REQUEST_END;
	static const char RESPONSE_END;

	/**
	 * @brief Default constructor.
	 */
	Controller();

	~Controller();

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
	inline bai::tcp::endpoint getEndpoint() const { return socket.local_endpoint(); }

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
	 * @brief Get a scan from LDS.
	 * @return ScanData
	 */
	ScanData getLdsScan();

	/**
	 * @brief Get the current scheduler time.
	 * @return "DayOfWeek HourOf24:Min:Sec" (example: "Sunday 13:57:09")
	 */
	std::string getTime();

	/**
	 * @brief Send a command to the Neato robot
	 * @param command A commmand with all parameter.
	 * @return The response to the command.
	 */
	std::string sendCommand(const std::string& command);
};

/**
 * @brief The NetworkException class is used for communication errors of the controller.
 */
class NetworkException : public std::runtime_error
{
public:
	NetworkException(const std::string& message);
};

}

#endif // NEATOC_CONTROLLER_HPP
