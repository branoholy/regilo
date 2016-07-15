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

#ifndef REGILO_CONTROLLER_HPP
#define REGILO_CONTROLLER_HPP

#include <sstream>

#include <boost/asio/io_service.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>

#include "log.hpp"
#include "utils.hpp"

namespace regilo {

namespace ba = boost::asio;

/**
 * @brief The IController interface is used for all controller classes.
 */
class IController
{
public:
	/**
	 * @brief Default destructor.
	 */
	virtual ~IController() = default;

	/**
	 * @brief Connect the controller to a device.
	 * @param endpoint The device endpoint (a path, IP address, etc.).
	 */
	virtual void connect(const std::string& endpoint) = 0;

	/**
	 * @brief Test if the controller is connected.
	 * @return True if connected.
	 */
	virtual bool isConnected() const = 0;

	/**
	 * @brief Get the endpoint of device.
	 * @return The device endpoint or empty string.
	 */
	virtual std::string getEndpoint() const = 0;

	/**
	 * @brief Get the current Log.
	 * @return The Log or empty std::shared_ptr.
	 */
	virtual std::shared_ptr<ILog> getLog() = 0;

	/**
	 * @brief Get the current Log (a const variant).
	 * @return The Log or empty std::shared_ptr.
	 */
	virtual std::shared_ptr<const ILog> getLog() const = 0;

	/**
	 * @brief Set a Log (it can be shared between more controllers).
	 * @param log Smart pointer to the Log.
	 */
	virtual void setLog(std::shared_ptr<ILog> log) = 0;

	/**
	 * @brief Send a command to the device.
	 * @param command A command with all parameters.
	 * @return A string response to the command.
	 */
	virtual std::string sendCommand(const std::string& command) = 0;
};

/**
 * @brief The StreamController class is used to communicate with a device.
 */
template<typename StreamT>
class StreamController : public virtual IController
{
private:
	ba::streambuf istreamBuffer;
	std::istream istream;

	ba::streambuf ostreamBuffer;
	std::ostream ostream;

protected:
	std::istringstream deviceOutput; ///< A buffer for the device output.
	std::ostringstream deviceInput; ///< A buffer for the device input.

	ba::io_service ioService; ///< The Boost IO service.
	StreamT stream; ///< A stream (TCP, socket, etc.) that is used for read/write operations.

	std::shared_ptr<Log> log; ///< A log that is connected to the controller.

	/**
	 * @brief Send a command from the device input to the device.
	 */
	template<typename Response = void, typename std::enable_if<std::is_void<Response>::value>::type* = nullptr>
	void sendCommand();

	/**
	 * @brief Send a command from the device input to the device.
	 * @return A response to the command.
	 */
	template<typename Response, typename std::enable_if<!std::is_void<Response>::value>::type* = nullptr>
	Response sendCommand();

public:
	typedef StreamT Stream; ///< The stream type for this Controller.

	std::string REQUEST_END = "\n"; ///< A string that the request ends with.
	std::string RESPONSE_END = "\n"; ///< A string that the response ends with.

	bool readResponse = true; ///< If true the sendCommand method reads a response.
	bool readCommand = true; ///< If true the input command is read from the response at first.

	/**
	 * @brief Default constructor.
	 */
	StreamController();

	/**
	 * @brief Controller with a log file specified by a path.
	 * @param logPath Path to the log file.
	 */
	StreamController(const std::string& logPath);

	/**
	 * @brief Controller with a log specified by a stream.
	 * @param logStream The log stream.
	 */
	StreamController(std::iostream& logStream);

	/**
	 * @brief Default destructor.
	 */
	virtual ~StreamController();

	virtual inline bool isConnected() const override { return stream.is_open(); }

	virtual inline std::shared_ptr<ILog> getLog() override { return log; }
	virtual inline std::shared_ptr<const ILog> getLog() const override { return log; }

	virtual void setLog(std::shared_ptr<ILog> log) override;

	virtual std::string sendCommand(const std::string& command) final override;

	/**
	 * @brief Send a command to the device.
	 * @param command A command with all parameters.
	 * @return A response to the command.
	 */
	template<typename Response = void, typename Command>
	Response sendCommand(const Command& command);

	/**
	 * @brief Send a command to the device.
	 * @param command A command without parameters.
	 * @param params Parameters that will be inserted into the command (separated by space).
	 * @return A response to the command.
	 */
	template<typename Response = void, typename Command, typename... Args>
	Response sendCommand(const Command& command, const Args& ... params);

	/**
	 * @brief Create a command with the specified parameters (printf formatting is used) and send it to the device.
	 * @param commandFormat A command format without parameters.
	 * @param params Parameters that will be inserted into the command.
	 * @return A response to the command.
	 */
	template<typename Response = void, typename... Args>
	Response sendFormattedCommand(const std::string& commandFormat, Args... params);

	/**
	 * @brief Create a command with the specified parameters (printf formatting is used).
	 * @param commandFormat A command format without parameters.
	 * @param params Parameters that will be inserted into the command.
	 * @return The command with the parameters.
	 */
	template<typename... Args>
	std::string createFormattedCommand(const std::string& commandFormat, Args... params) const;
};

template<typename StreamT>
StreamController<StreamT>::StreamController() :
	istream(&istreamBuffer),
	ostream(&ostreamBuffer),
	stream(ioService)
{
}

template<typename StreamT>
StreamController<StreamT>::StreamController(const std::string& logPath) : StreamController()
{
	if(!logPath.empty())
	{
		log.reset(new Log(logPath));
	}
}

template<typename StreamT>
StreamController<StreamT>::StreamController(std::iostream& logStream) : StreamController()
{
	this->log.reset(new Log(logStream));
}

template<typename StreamT>
StreamController<StreamT>::~StreamController()
{
	if(stream.is_open()) stream.close();
}

template<typename StreamT>
void StreamController<StreamT>::setLog(std::shared_ptr<ILog> log)
{
	std::shared_ptr<Log> logPointer = std::dynamic_pointer_cast<Log>(log);
	this->log.swap(logPointer);
}

template<typename StreamT>
std::string StreamController<StreamT>::sendCommand(const std::string& command)
{
	return sendCommand<std::string>(command);
}

template<typename StreamT>
template<typename Response, typename Command>
Response StreamController<StreamT>::sendCommand(const Command& command)
{
	deviceInput << command;
	return sendCommand<Response>();
}

template<typename StreamT>
template<typename Response, typename Command, typename... Args>
Response StreamController<StreamT>::sendCommand(const Command& command, const Args& ... params)
{
	deviceInput << command << ' ';
	return sendCommand<Response>(params...);
}

template<typename StreamT>
template<typename Response, typename... Args>
Response StreamController<StreamT>::sendFormattedCommand(const std::string& commandFormat, Args... params)
{
	return sendCommand<Response>(createFormattedCommand(commandFormat, params...));
}

template<typename StreamT>
template<typename Response, typename std::enable_if<std::is_void<Response>::value>::type*>
void StreamController<StreamT>::sendCommand()
{
	deviceInput << REQUEST_END;

	std::string input = deviceInput.str();
	ostream << input;

	ba::write(stream, ostreamBuffer);
	ostream.flush();

	deviceInput.clear();
	deviceInput.str("");

	std::string output;
	if(readResponse)
	{
		ba::read_until(stream, istreamBuffer, RESPONSE_END);

		if(readCommand)
		{
			std::string cmdInput;
			getLine(istream, cmdInput, REQUEST_END);
			cmdInput = cmdInput.substr(0, cmdInput.length() - REQUEST_END.length());
		}

		getLine(istream, output, RESPONSE_END);
		deviceOutput.clear();
		deviceOutput.str(output);
	}

	if(log != nullptr) log->write(input, output);
}

template<typename StreamT>
template<typename Response, typename std::enable_if<!std::is_void<Response>::value>::type*>
Response StreamController<StreamT>::sendCommand()
{
	sendCommand();

	Response output;
	deviceOutput >> output;

	return output;
}

template<typename StreamT>
template<typename... Args>
std::string StreamController<StreamT>::createFormattedCommand(const std::string& command, Args... params) const
{
	std::size_t size = std::snprintf(nullptr, 0, command.c_str(), params...) + 1;
	char *buffer = new char[size];
	std::snprintf(buffer, size, command.c_str(), params...);

	std::string result(buffer, buffer + size - 1);
	delete[] buffer;

	return result;
}

}

#endif // REGILO_CONTROLLER_HPP
