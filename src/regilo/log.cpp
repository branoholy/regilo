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

#include "regilo/log.hpp"

namespace regilo {

Log::Log(const std::string& filePath) :
	filePath(filePath),
	fileStream(new std::fstream(filePath, std::fstream::in | std::fstream::out | std::fstream::app)),
	stream(*fileStream)
{
}

Log::Log(std::iostream& stream) :
	fileStream(nullptr),
	stream(stream)
{
}

Log::~Log()
{
	delete fileStream;
}

void Log::readMetadata(std::istream& metaStream)
{
	metaStream >> version;
}

void Log::writeMetadata(std::ostream& metaStream)
{
	metaStream << version;
}

std::string Log::read()
{
	std::string command;
	return read(command);
}

std::string Log::read(std::string& logCommand)
{
	streamMutex.lock();

	if(!metadataRead)
	{
		std::string metaData;
		std::getline(stream, metaData, MESSAGE_END);
		std::istringstream metaStream(metaData);

		readMetadata(metaStream);
		metadataRead = true;
	}

	std::string response;

	std::getline(stream, logCommand, MESSAGE_END);
	std::getline(stream, response, MESSAGE_END);

	streamMutex.unlock();

	return response;
}

std::string Log::readCommand(const std::string& command)
{
	std::string logCommand;
	return readCommand(command, logCommand);
}

std::string Log::readCommand(const std::string& command, std::string& logCommand)
{
	std::string response;
	do
	{
		response = read(logCommand);
	}
	while(!(boost::algorithm::starts_with(logCommand, command) || isEnd()));

	return response;
}

void Log::write(const std::string& command, const std::string& response)
{
	streamMutex.lock();

	if(!metadataWritten)
	{
		std::ostringstream metaStream;
		writeMetadata(metaStream);

		stream << metaStream.str() << MESSAGE_END;
		metadataWritten = true;
	}

	stream << command << MESSAGE_END;
	stream << response << MESSAGE_END;

	streamMutex.unlock();
}

void Log::close()
{
	if(fileStream != nullptr) fileStream->close();
}

template class TimedLog<std::chrono::nanoseconds>;
template class TimedLog<std::chrono::microseconds>;
template class TimedLog<std::chrono::milliseconds>;
template class TimedLog<std::chrono::seconds>;

}
