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

#include <fstream>
#include <sstream>

#include <boost/asio/io_service.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>

#include "log.hpp"
#include "scandata.hpp"

namespace neatoc {

namespace ba = boost::asio;

/**
 * @brief The Controller class is the interface for all controller classes.
 */
class Controller
{
public:
	/**
	 * @brief Default destructor.
	 */
	virtual ~Controller() = default;

	/**
	 * @brief Connect the controller to the device.
	 * @param endpoint The endpoint of device.
	 */
	virtual void connect(const std::string& endpoint) = 0;

	/**
	 * @brief Get the path of log file if the controller was created with a log path otherwise the empty string.
	 * @return The path or empty string.
	 */
	virtual std::string getLogPath() const = 0;

	/**
	 * @brief Get the endpoint of device.
	 * @return The endpoint.
	 */
	virtual std::string getEndpoint() const = 0;

	/**
	 * @brief Get a scan from the device.
	 * @param fromDevice Specify if you want to get a scan from the device (true) or log (false). Default: true
	 * @return ScanData
	 */
	virtual ScanData getScan(bool fromDevice = true) = 0;
};

/**
 * @brief The BaseController class is used to communicate with a device.
 */
template<typename Stream>
class BaseController : public Controller
{
private:
	ba::streambuf istreamBuffer;
	std::istream istream;

	ba::streambuf ostreamBuffer;
	std::ostream ostream;

	static void getLine(std::istream& stream, std::string& line, const std::string& delim);

protected:
	std::istringstream deviceOutput;
	std::ostringstream deviceInput;

	std::size_t lastScanId;
	ba::io_service ioService;
	Stream stream;

	Log *log;
	std::string logPath;
	std::fstream *logFile;

	virtual std::string sendCommand() final;
	virtual std::string getScanCommand() const = 0;
	virtual bool parseScanData(std::istream& in, ScanData& data) = 0;

public:
	typedef Stream StreamType;

	std::string REQUEST_END;
	std::string RESPONSE_END;

	/**
	 * @brief Default constructor.
	 */
	BaseController();

	/**
	 * @brief Controller with a log file specified by a path.
	 * @param logPath Path to the log file.
	 */
	BaseController(const std::string& logPath);

	/**
	 * @brief Controller with a log specified by a stream.
	 * @param logStream The log stream.
	 */
	BaseController(std::iostream& logStream);

	/**
	 * @brief Default destructor.
	 */
	virtual ~BaseController();

	/**
	 * @brief Get the path of log file if the controller was created with a log path otherwise the empty string.
	 * @return The path or empty string.
	 */
	virtual inline std::string getLogPath() const { return logPath; }

	/**
	 * @brief Get a scan from the device.
	 * @param fromDevice Specify if you want to get a scan from the device (true) or log (false). Default: true
	 * @return ScanData
	 */
	virtual ScanData getScan(bool fromDevice = true) final;

	/**
	 * @brief Send a command to the device.
	 * @param command A commmand with all parameter.
	 * @return A response to the command.
	 */
	virtual std::string sendCommand(const std::string& command) final;

	/**
	 * @brief Create a command with the specified parameters (printf formatting is used).
	 * @param command A command without parameters.
	 * @param params Parameters that will be inserted into the command.
	 * @return The command with the parameters.
	 */
	template<typename... Args>
	std::string createCommand(const std::string& command, Args... params) const;

	/**
	 * @brief createAndSendCommand Create a command with the specified parameters (printf formatting is used) and send it to the device.
	 * @param command A command without parameters.
	 * @param params Parameters that will be inserted into the command.
	 * @return A response to the command.
	 */
	template<typename... Args>
	std::string createCommandAndSend(const std::string& command, Args... params);
};

template<typename Stream>
BaseController<Stream>::BaseController() :
	istream(&istreamBuffer),
	ostream(&ostreamBuffer),
	lastScanId(0),
	stream(ioService),
	log(nullptr), logFile(nullptr),
	REQUEST_END("\n"),
	RESPONSE_END("\n")
{
}

template<typename Stream>
BaseController<Stream>::BaseController(const std::string& logPath) : BaseController()
{
	if(logPath.length() > 0)
	{
		logFile = new std::fstream(logPath, std::fstream::in | std::fstream::out | std::fstream::app);
		log = new Log(*logFile);
		this->logPath = logPath;
	}
}

template<typename Stream>
BaseController<Stream>::BaseController(std::iostream& logStream) : BaseController()
{
	this->log = new Log(logStream);
}

template<typename Stream>
BaseController<Stream>::~BaseController()
{
	if(log != nullptr) delete log;
	if(logFile != nullptr) delete logFile;
	if(stream.is_open()) stream.close();
}

template<typename Stream>
ScanData BaseController<Stream>::getScan(bool fromDevice)
{
	ScanData data;

	if(fromDevice)
	{
		sendCommand(getScanCommand());
		parseScanData(deviceOutput, data);
	}
	else
	{
		std::istringstream response(log->readCommand(getScanCommand()));
		parseScanData(response, data);
	}
	if(!data.empty()) data.scanId = lastScanId++;

	return data;
}

template<typename Stream>
std::string BaseController<Stream>::sendCommand(const std::string& command)
{
	deviceInput << command;
	return sendCommand();
}

template<typename Stream>
std::string BaseController<Stream>::sendCommand()
{
	deviceInput << REQUEST_END;

	std::string input = deviceInput.str();
	ostream << input;
	ba::write(stream, ostreamBuffer);
	deviceInput.clear();
	deviceInput.str("");

	ba::read_until(stream, istreamBuffer, RESPONSE_END);

	std::string cmdInput;
	getLine(istream, cmdInput, REQUEST_END);
	cmdInput.pop_back();

	std::string output;
	getLine(istream, output, RESPONSE_END);
	deviceOutput.clear();
	deviceOutput.str(output);

	if(log != nullptr) log->write(input, output);

	return output;
}

template<typename Stream>
void BaseController<Stream>::getLine(std::istream& stream, std::string& line, const std::string& delim)
{
	if(delim.empty()) std::getline(stream, line);
	else if(delim.size() == 1) std::getline(stream, line, delim.front());
	else
	{
		std::string delimPart;
		while(stream)
		{
			char ch = stream.get();
			if(ch == delim.at(delimPart.size()))
			{
				delimPart += ch;
				if(delimPart.size() == delim.size()) break;
			}
			else
			{
				if(!delimPart.empty())
				{
					line += delimPart;
					delimPart.clear();
				}

				line += ch;
			}
		}
	}
}

template<typename Stream>
template<typename... Args>
std::string BaseController<Stream>::createCommand(const std::string& command, Args... params) const
{
	std::size_t size = std::snprintf(nullptr, 0, command.c_str(), params...) + 1;
	char* buffer = new char[size];
	std::snprintf(buffer, size, command.c_str(), params...);

	std::string result(buffer, buffer + size - 1);
	delete[] buffer;

	return result;
}

template<typename Stream>
template<typename... Args>
std::string BaseController<Stream>::createCommandAndSend(const std::string& command, Args... params)
{
	return sendCommand(createCommand(command, params...));
}

}

#endif // NEATOC_CONTROLLER_HPP
