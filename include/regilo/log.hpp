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

#ifndef REGILO_LOG_HPP
#define REGILO_LOG_HPP

#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <thread>

#include <boost/algorithm/string/predicate.hpp>

#include "regilo/utils.hpp"

namespace regilo {

class Log
{
public:
	/**
	 * @brief Default destructor.
	 */
	virtual ~Log() = default;

	/**
	 * @brief Get the path of file if the log was created with a path otherwise the empty string.
	 * @return The path or empty string.
	 */
	virtual const std::string& getFilePath() const = 0;

	/**
	 * @brief Get the current underlying stream.
	 * @return The underlying stream
	 */
	virtual std::iostream& getStream() = 0;

	/**
	 * @brief Test if the stream is EOF.
	 * @return true/false
	 */
	virtual bool isEnd() const = 0;

	/**
	 * @brief Read one command from the log.
	 * @return The response of the command.
	 */
	virtual std::string read() = 0;

	/**
	 * @brief Read one command from the log.
	 * @param logCommand The input of the command that was read.
	 * @return The response of the command.
	 */
	virtual std::string read(std::string& logCommand) = 0;

	/**
	 * @brief Read specified command from the log (the others are skipped).
	 * @param command The command to read (The boost::algorithm::starts_with() method is used to compare).
	 * @return The response of the command.
	 */
	virtual std::string readCommand(const std::string& command) = 0;

	/**
	 * @brief Read specified command from the log (the others are skipped).
	 * @param command The command to read (The boost::algorithm::starts_with() method is used to compare).
	 * @param logCommand The input of the command that was read.
	 * @return The response of the command.
	 */
	virtual std::string readCommand(const std::string& command, std::string& logCommand) = 0;

	/**
	 * @brief Write a command and response to the log.
	 * @param command The command (with all parameters).
	 * @param response The response of the command.
	 */
	virtual void write(const std::string& command, const std::string& response) = 0;
};

class BasicLog : public Log
{
private:
	std::string filePath;
	std::fstream *fileStream;

protected:
	std::iostream& stream;

public:
	char MESSAGE_END = '$';

	/**
	 * @brief Log constructor with logging to a file.
	 * @param filePath The path of file
	 */
	BasicLog(const std::string& filePath);

	/**
	 * @brief Log constructor with logging to a stream.
	 * @param stream Input/output stream
	 */
	BasicLog(std::iostream& stream);

	virtual ~BasicLog();

	virtual inline const std::string& getFilePath() const override { return filePath; }
	virtual inline std::iostream& getStream() override { return stream; }
	virtual inline bool isEnd() const override { return !stream; }

	virtual std::string read() override;
	virtual std::string read(std::string& logCommand) override;
	virtual std::string readCommand(const std::string& command) override;
	virtual std::string readCommand(const std::string& command, std::string& logCommand) override;

	virtual void write(const std::string& command, const std::string& response) override;
};

/**
 * @brief The Log class is used to log all commands that were send to the device.
 */
template<typename Duration = std::chrono::milliseconds>
class TimedLog : public BasicLog
{
private:
	typename Duration::rep commandTime;

	typename Duration::rep firstReadTime;
	typename Duration::rep firstCommandTime = std::numeric_limits<typename Duration::rep>::max();

public:
	using BasicLog::BasicLog;

	virtual ~TimedLog() = default;

	/**
	 * @brief Get last command time (after reading).
	 * @return Time since epoch in Duration units
	 */
	inline typename Duration::rep getLastCommandTime() const { return commandTime; }

	/**
	 * @brief Sync command times with real time. It means that all read methods will block
	 *        its execution until the current time is bigger than the command time.
	 */
	inline void syncTime(bool sync = true) { firstCommandTime = (sync ? -1 : std::numeric_limits<long>::max()); }

	virtual std::string read(std::string& logCommand) override;
	virtual void write(const std::string& command, const std::string& response) override;
};

template<typename Duration>
std::string TimedLog<Duration>::read(std::string& logCommand)
{
	std::string epochTime;
	std::getline(stream, epochTime, MESSAGE_END);
	std::istringstream epochStream(epochTime);
	epochStream >> commandTime;

	std::string response = BasicLog::read(logCommand); // TODO: Swap with time

	if(firstCommandTime == -1)
	{
		firstReadTime = epoch<Duration>();
		firstCommandTime = commandTime;
	}
	else
	{
		typename Duration::rep elapsed = epoch<Duration>() - firstReadTime;
		typename Duration::rep elapsedLog = commandTime - firstCommandTime;

		while(elapsed < elapsedLog)
		{
			std::this_thread::sleep_for(Duration(elapsedLog - elapsed));
			elapsed = epoch<Duration>() - firstReadTime;
		}
	}

	return response;
}

template<typename Duration>
void TimedLog<Duration>::write(const std::string& command, const std::string& response)
{
	stream << epoch<Duration>() << MESSAGE_END;

	BasicLog::write(command, response); // TODO: Swap with time
}

}

#endif // REGILO_LOG_HPP
