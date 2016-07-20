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

#include <fstream>
#include <iostream>
#include <mutex>

#include "regilo/utils.hpp"

namespace regilo {

/**
 * @brief The InvalidLogException is thrown when the data stream contains
 *		  a value that is not expected.
 */
class InvalidLogException : public std::runtime_error
{
public:
	/**
	 * @brief The default constructor.
	 * @param message A string that is printed.
	 */
	InvalidLogException(const std::string& message);
};

/**
 * @brief The LogMetadata class stores metadata of a Log.
 */
class LogMetadata
{
private:
	std::string type = "log";
	int version = 2;

protected:
	/**
	 * @brief The default constructor is available only in derived and friend classes.
	 */
	LogMetadata() = default;

	/**
	 * @brief This constructor is available only in derived and friend classes.
	 * @param type A log type.
	 * @param version A log version.
	 */
	LogMetadata(const std::string& type, int version);

public:
	/**
	 * @brief The default destructor.
	 */
	virtual ~LogMetadata() = default;

	/**
	 * @brief Get the log type.
	 * @return A string with the type.
	 */
	inline virtual const std::string& getType() const final { return type; }

	/**
	 * @brief Get the log version.
	 * @return The version number.
	 */
	inline virtual int getVersion() const final { return version; }

	friend class Log;
};

/**
 * @brief The ILog interface has to be implemented in all Log classes.
 */
class ILog
{
public:
	/**
	 * @brief Default destructor.
	 */
	virtual ~ILog() = default;

	/**
	 * @brief Get the path of file if the log was created with a path otherwise the empty string.
	 * @return The path or empty string.
	 */
	virtual const std::string& getFilePath() const = 0;

	/**
	 * @brief Get the current underlying stream.
	 * @return The underlying stream.
	 */
	virtual std::iostream& getStream() = 0;

	/**
	 * @brief Test if the stream is EOF.
	 * @return True if the log is in the EOF state.
	 */
	virtual bool isEnd() const = 0;

	/**
	 * @brief Read the metadata. It is usefull to determinate the metadata in runtime.
	 */
	virtual void readMetadata() = 0;

	/**
	 * @brief Get the associated metadata.
	 * @return A pointer to metadata
	 */
	virtual const LogMetadata* getMetadata() const = 0;

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

	/**
	 * @brief Close the log file.
	 */
	virtual void close() = 0;
};

/**
 * @brief The Log class is a basic log with a simple read/write functionality.
 *		  It is used to log all commands that were send to the device.
 */
class Log : public virtual ILog
{
private:
	std::string filePath;
	std::fstream *fileStream;

	std::mutex streamMutex;

	bool metadataRead = false;
	bool metadataWritten = false;

protected:
	std::iostream& stream; ///< The underlying stream.
	LogMetadata *metadata = nullptr; ///< The metadata object.

	/**
	 * @brief Read the next char and check if it matches.
	 *
	 * Read the next char and check if it matches with the name. If not
	 * an exception is thrown.
	 *
	 * @param stream A stream that is used for reading.
	 * @param name A char that is used to check.
	 */
	void readName(std::istream& stream, char name);

	/**
	 * @brief Read the next string and check if it matches.
	 *
	 * Read the next string and check if it matches with the name. If not
	 * an exception is thrown.
	 *
	 * @param stream A stream that is used for reading.
	 * @param name A string that is used to check.
	 */
	void readName(std::istream& stream, const std::string& name);

	/**
	 * @brief Read the next value that is specified with a length.
	 * @param stream A stream that is used for reading.
	 * @return The value.
	 */
	std::string readValue(std::istream& stream);

	/**
	 * @brief Read the next char and value that is specified with a length.
	 *
	 * Read the next char and value that is specified with a length and check
	 * if the char matches with the name. If not an exception is thrown.
	 *
	 * @param stream A stream that is used for reading.
	 * @param name A char that is used to check.
	 * @return The value.
	 */
	std::string readNameValue(std::istream& stream, char name);

	/**
	 * @brief Read the next char and value that is specified with a length.
	 *
	 * Read the next char and value that is specified with a length and check
	 * if the char matches with the name. If not an exception is thrown.
	 *
	 * @param stream A stream that is used for reading.
	 * @param name A string that is used to check.
	 * @return The value.
	 */
	std::string readNameValue(std::istream& stream, const std::string& name);

	/**
	 * @brief Read metadata from the log.
	 * @param metaStream A stream that is used for reading.
	 */
	virtual void readMetadata(std::istream& metaStream);

	/**
	 * @brief Write metadata to the log.
	 * @param metaStream A stream that is used for writing.
	 */
	virtual void writeMetadata(std::ostream& metaStream);

	/**
	 * @brief Read a command and response from the log.
	 *		  This method can be safely overridden.
	 * @param logCommand The input of the command that was read.
	 * @return The response of the command.
	 */
	virtual std::string readData(std::string& logCommand);

	/**
	 * @brief Write a command and response to the log.
	 *		  This method can be safely overridden.
	 * @param command The command (with all parameters).
	 * @param response The response of the command.
	 */
	virtual void writeData(const std::string& command, const std::string& response);

public:
	/**
	 * @brief Log constructor with logging to a file.
	 * @param filePath The path of file.
	 */
	Log(const std::string& filePath);

	/**
	 * @brief Log constructor with logging to a stream.
	 * @param stream Input/output stream.
	 */
	Log(std::iostream& stream);

	/**
	 * @brief Default destructor.
	 */
	virtual ~Log();

	virtual inline const std::string& getFilePath() const override { return filePath; }
	virtual inline std::iostream& getStream() override { return stream; }
	virtual inline bool isEnd() const override { return !stream; }

	virtual void readMetadata() override final;
	virtual inline const LogMetadata* getMetadata() const override { return metadata; }

	virtual std::string read() override final;
	virtual std::string read(std::string& logCommand) override final;
	virtual std::string readCommand(const std::string& command) override final;
	virtual std::string readCommand(const std::string& command, std::string& logCommand) override final;

	virtual void write(const std::string& command, const std::string& response) override final;

	virtual void close() override;
};

}

#endif // REGILO_LOG_HPP
