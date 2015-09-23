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

#include "neatoc/log.hpp"

#include <chrono>

#include <boost/algorithm/string.hpp>

char neatoc::Log::MESSAGE_END = '$';

neatoc::Log::Log(std::iostream& stream) :
	stream(stream)
{
}

std::string neatoc::Log::read()
{
	std::string command;
	return read(command);
}

std::string neatoc::Log::read(std::string& logCommand)
{
	std::string secondsEpoch, response;

	std::getline(stream, secondsEpoch, MESSAGE_END);
	std::getline(stream, logCommand, MESSAGE_END);
	std::getline(stream, response, MESSAGE_END);

	return response;
}

std::string neatoc::Log::readCommand(const std::string& command)
{
	std::string logCommand;
	return readCommand(command, logCommand);
}

std::string neatoc::Log::readCommand(const std::string& command, std::string& logCommand)
{
	std::string response;
	do
	{
		response = read(logCommand);
	}
	while(!(boost::algorithm::starts_with(logCommand, command) || isEnd()));

	return response;
}

void neatoc::Log::write(const std::string& command, const std::string& response)
{
	auto sinceEpoch = std::chrono::system_clock::now().time_since_epoch();
	long secondsEpoch = sinceEpoch.count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den;

	stream << secondsEpoch << MESSAGE_END;
	stream << command << MESSAGE_END;
	stream << response << MESSAGE_END;
}
