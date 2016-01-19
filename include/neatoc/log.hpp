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

#ifndef NEATOC_LOG_HPP
#define NEATOC_LOG_HPP

#include <iostream>

namespace neatoc {

/**
 * @brief The Log class is used to log all commands that were send to the Neato robot.
 */
class Log
{
private:
	std::iostream& stream;

public:
	static char MESSAGE_END;

	/**
	 * @brief Log constructor
	 * @param stream Input/output stream
	 */
	Log(std::iostream& stream);

	/**
	 * @brief Test if the stream is EOF.
	 * @return true/false
	 */
	inline bool isEnd() const { return !stream; }

	/**
	 * @brief Read one command from the log.
	 * @return The response of the command.
	 */
	std::string read();

	/**
	 * @brief Read one command from the log.
	 * @param logCommand The input of the command that was read.
	 * @return The response of the command.
	 */
	std::string read(std::string& logCommand);

	/**
	 * @brief Read specified command from the log (the others are skipped).
	 * @param command The command to read (The boost::algorithm::starts_with() method is used to compare).
	 * @return The response of the command.
	 */
	std::string readCommand(const std::string& command);

	/**
	 * @brief Read specified command from the log (the others are skipped).
	 * @param command The command to read (The boost::algorithm::starts_with() method is used to compare).
	 * @param logCommand The input of the command that was read.
	 * @return The response of the command.
	 */
	std::string readCommand(const std::string& command, std::string& logCommand);

	/**
	 * @brief Write a command and response to the log.
	 * @param command The command (with all parameters).
	 * @param response The response of the command.
	 */
	void write(const std::string& command, const std::string& response);

};

}

#endif // NEATOC_LOG_HPP
