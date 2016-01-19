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

#ifndef NEATOC_CONTROLLER_HPP
#define NEATOC_CONTROLLER_HPP

#include <cstdio>
#include <sstream>

#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "scandata.hpp"

namespace neatoc {

namespace bai = boost::asio::ip;

class Log;

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

	Log *log;
	std::string logPath;
	std::fstream *logFile;

	std::string sendCommand();

public:
	static std::string ON;
	static std::string OFF;
	static std::string LDS_SCAN_HEADER;
	static std::string LDS_SCAN_FOOTER;

	static std::string TEST_MODE;
	static std::string SET_LDS_ROTATION;
	static std::string SET_MOTOR;
	static std::string GET_TIME;
	static std::string GET_LDS_SCAN;

	static char REQUEST_END;
	static char RESPONSE_END;

	/**
	 * @brief Default constructor.
	 */
	Controller();

	/**
	 * @brief Controller with a log file specified by a path.
	 * @param logPath Path to the log file.
	 */
	Controller(const std::string& logPath);

	/**
	 * @brief Controller with a log specified by a stream.
	 * @param logStream The log stream.
	 */
	Controller(std::iostream& logStream);

	/**
	 * @brief Default destructor.
	 */
	virtual ~Controller();

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
	inline bai::tcp::endpoint getEndpoint() const { return socket.remote_endpoint(); }

	/**
	 * @brief Get the path of log file if the controller was created with a log path otherwise the empty string.
	 * @return The path or empty string.
	 */
	inline std::string getLogPath() const { return logPath; }

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
	 * @param fromScanner Specify if you want to get a scan from Neato scanner (true) or log (false). Default: true
	 * @return ScanData
	 */
	ScanData getLdsScan(bool fromScanner = true);

	/**
	 * @brief Get the current scheduler time.
	 * @return "DayOfWeek HourOf24:Min:Sec" (example: "Sunday 13:57:09")
	 */
	std::string getTime();

	/**
	 * @brief Send a command to the Neato robot
	 * @param command A commmand with all parameter.
	 * @return A response to the command.
	 */
	std::string sendCommand(const std::string& command);

	/**
	 * @brief Create a command with the specified parameters (printf formatting is used).
	 * @param command A command without parameters (e.g. neatoc::Controller::SET_MOTOR).
	 * @param params Parameters that will be inserted into the command.
	 * @return The command with the parameters.
	 */
	template<typename... Args>
	std::string createCommand(const std::string& command, Args... params);

	/**
	 * @brief createAndSendCommand Create a command with the specified parameters (printf formatting is used) and send it to the Neato robot.
	 * @param command A command without parameters (e.g. neatoc::Controller::SET_MOTOR).
	 * @param params Parameters that will be inserted into the command.
	 * @return A response to the command.
	 */
	template<typename... Args>
	std::string createAndSendCommand(const std::string& command, Args... params);
};

template<typename... Args>
std::string Controller::createCommand(const std::string& command, Args... params)
{
	std::size_t size = std::snprintf(nullptr, 0, command.c_str(), params...) + 1;
	char* buffer = new char[size];
	std::snprintf(buffer, size, command.c_str(), params...);

	std::string result(buffer, buffer + size - 1);
	delete[] buffer;

	return result;
}

template<typename... Args>
std::string Controller::createAndSendCommand(const std::string& command, Args... params)
{
	return sendCommand(createCommand(command, params...));
}

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
