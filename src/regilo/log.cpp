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

#include <sstream>

#include <boost/algorithm/string/predicate.hpp>

namespace regilo {

InvalidLogException::InvalidLogException(const std::string& message) : std::runtime_error(message)
{
}

LogMetadata::LogMetadata(const std::string& type, int version) :
	type(type), version(version)
{
}

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
	delete metadata;
}

void Log::readName(std::istream& stream, char name)
{
	char logName;
	stream >> logName;

	if(stream)
	{
		if(logName == '#')
		{
			std::string comment;
			std::getline(stream, comment);

			readName(stream, name);
		}
		else if(name != logName) throw InvalidLogException('\'' + std::string(1, logName) + "' found but '" + name + "' expected.");
	}
}

void Log::readName(std::istream& stream, const std::string& name)
{
	std::string logName;
	stream >> logName;

	if(stream)
	{
		if(logName.front() == '#')
		{
			std::string comment;
			std::getline(stream, comment);

			readName(stream, name);
		}
		else if(name != logName) throw InvalidLogException('\'' + logName + "' found but '" + name + "' expected.");
	}
}

std::string Log::readValue(std::istream& stream)
{
	std::size_t length;
	stream >> length;

	// Read the space between length and data.
	stream.get();

	std::string value;
	if(stream)
	{
		char *data = new char[length];
		stream.read(data, length);

		value = std::string(data, length);
		delete[] data;
	}

	return value;
}

std::string Log::readNameValue(std::istream& stream, char name)
{
	readName(stream, name);
	return readValue(stream);
}

std::string Log::readNameValue(std::istream& stream, const std::string& name)
{
	readName(stream, name);
	return readValue(stream);
}

void Log::readMetadata()
{
	if(!metadataRead)
	{
		std::string metaLine;
		std::stringstream metaStream;
		while(std::getline(stream, metaLine) && !metaLine.empty())
		{
			if(metaLine.front() == '#') continue;

			metaStream << metaLine << std::endl;
		}

		readMetadata(metaStream);
		metadataRead = true;
	}
}

void Log::readMetadata(std::istream& metaStream)
{
	if(metadata == nullptr) metadata = new LogMetadata();

	readName(metaStream, "type");
	metaStream >> metadata->type;

	readName(metaStream, "version");
	metaStream >> metadata->version;
}

void Log::writeMetadata(std::ostream& metaStream)
{
	if(metadata == nullptr) metadata = new LogMetadata();

	metaStream << "type " << metadata->type << std::endl;
	metaStream << "version " << metadata->version << std::endl;
}

std::string Log::readData(std::string& logCommand)
{
	logCommand = readNameValue(stream, 'c');
	std::string response = readNameValue(stream, 'r');

	return response;
}

std::string Log::read()
{
	std::string logCommand;
	return read(logCommand);
}

std::string Log::read(std::string& logCommand)
{
	std::lock_guard<std::mutex> streamLock(streamMutex);

	readMetadata();

	std::string response = readData(logCommand);

	std::string responseEnd;
	while(std::getline(stream, responseEnd) && !responseEnd.empty())
	{
		if(responseEnd.front() != '#') break;
	}

	if(!responseEnd.empty()) throw InvalidLogException("Missing a new line after data.");

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

void Log::writeData(const std::string& command, const std::string& response)
{
	stream << "c " << command.length() << ' ' << command << std::endl;

	stream << "r " << response.length();
	if(!response.empty()) stream << ' ' << response;
	stream << std::endl;
}

void Log::write(const std::string& command, const std::string& response)
{
	std::lock_guard<std::mutex> streamLock(streamMutex);

	if(!metadataWritten)
	{
		std::ostringstream metaStream;
		writeMetadata(metaStream);

		stream << metaStream.str() << std::endl;
		metadataWritten = true;
	}

	writeData(command, response);
	stream << std::endl;
}

void Log::close()
{
	if(fileStream != nullptr) fileStream->close();
}

}
