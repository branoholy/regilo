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

#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "regilo/timedlog.hpp"

class StringNameLog : public regilo::Log
{
public:
	using Log::Log;
	virtual ~StringNameLog() = default;

	virtual std::string readData(std::string& logCommand);
};

std::string StringNameLog::readData(std::string& logCommand)
{
	logCommand = readNameValue(stream, "command");
	std::string response = readNameValue(stream, "response");

	return response;
}

struct LogFixture
{
	std::string logPath = "data/hokuyo-log.txt";
	std::string timedLogPath = "data/hokuyo-timed-log.txt";
	std::stringstream logStream;
	std::stringstream timedLogStream;
	std::stringstream commentStream;
	std::stringstream stringNameStream;

	regilo::Log *fileLog;
	regilo::Log *streamLog;
	regilo::Log *commentLog;
	regilo::Log *stringNameLog;

	regilo::TimedLog<std::chrono::nanoseconds> *timedFileLog;
	regilo::TimedLog<std::chrono::nanoseconds> *timedStreamLog;

	std::vector<regilo::ILog*> logs;

	LogFixture() :
		logStream("type log\nversion 2\n\nc 10 G00076801\n\nr 22 0\n0C0C0C0C0C0C0C0C0C0C\n\nc 2 V\n\nr 10 0\nVERSION1\n\nc 10 G00076801\n\nr 22 0\n0C0C0C0C0C0C0C0C0C0C\n\nc 2 V\n\nr 10 0\nVERSION2\n\n"),
		timedLogStream("type timedlog\nversion 2\ntimeres 1 1000000000\n\nc 10 G00076801\n\nr 22 0\n0C0C0C0C0C0C0C0C0C0C\nt 103203758\n\nc 2 V\n\nr 10 0\nVERSION1\nt 103203759\n\nc 10 G00076801\n\nr 22 0\n0C0C0C0C0C0C0C0C0C0C\nt 103203760\n\nc 2 V\n\nr 10 0\nVERSION2\nt 103203761\n\n"),
		commentStream("# First line comment\ntype log\n# Comment in metadata\nversion 2\n\nc 2 V\n\nr 2 2\n\n\n"),
		stringNameStream("type stringlog\nversion 2\n\ncommand 8 getscan\n\nresponse 2 2\n\n\n"),
		fileLog(new regilo::Log(logPath)),
		streamLog(new regilo::Log(logStream)),
		commentLog(new regilo::Log(commentStream)),
		stringNameLog(new StringNameLog(stringNameStream)),
		timedFileLog(new regilo::TimedLog<std::chrono::nanoseconds>(timedLogPath)),
		timedStreamLog(new regilo::TimedLog<std::chrono::nanoseconds>(timedLogStream))
	{
		logs.push_back(fileLog);
		logs.push_back(streamLog);
		logs.push_back(timedFileLog);
		logs.push_back(timedStreamLog);
	}

	~LogFixture()
	{
		for(regilo::ILog *log : logs)
		{
			delete log;
		}
	}
};

BOOST_AUTO_TEST_SUITE(LogSuite)

BOOST_FIXTURE_TEST_CASE(LogConstructorValues, LogFixture)
{
	BOOST_CHECK(fileLog->getFilePath() == logPath);
	BOOST_CHECK(&streamLog->getStream() == &logStream);
	BOOST_CHECK(timedFileLog->getFilePath() == timedLogPath);
	BOOST_CHECK(&timedStreamLog->getStream() == &timedLogStream);
}

BOOST_FIXTURE_TEST_CASE(LogMetadata, LogFixture)
{
	std::string types[] = { "log", "timedlog" };

	for(std::size_t i = 0; i < 2; i++)
	{
		regilo::ILog *log = logs[2 * i + 1];

		BOOST_CHECK(log->getMetadata() == nullptr);

		log->readMetadata();
		const regilo::LogMetadata *metadata = log->getMetadata();
		BOOST_REQUIRE(metadata != nullptr);

		BOOST_CHECK_EQUAL(metadata->getType(), types[i]);
		BOOST_CHECK_EQUAL(metadata->getVersion(), 2);
	}
}

BOOST_FIXTURE_TEST_CASE(LogRead, LogFixture)
{
	for(regilo::ILog *log : logs)
	{
		BOOST_REQUIRE(log->getStream());
		BOOST_CHECK_EQUAL(bool(log->getStream()), !log->isEnd());

		std::string logCommand;
		std::string logResponse = log->read(logCommand);

		BOOST_CHECK_EQUAL(logCommand, "G00076801\n");

		BOOST_CHECK_EQUAL(logResponse.substr(0, 22), "0\n0C0C0C0C0C0C0C0C0C0C");

		regilo::TimedLog<std::chrono::nanoseconds> *timedLog = dynamic_cast<regilo::TimedLog<std::chrono::nanoseconds>*>(log);
		if(timedLog != nullptr)
		{
			BOOST_CHECK(timedLog->getLastCommandNanoseconds() == std::chrono::nanoseconds(103203758));
			BOOST_CHECK(timedLog->getLastCommandTime() == std::chrono::nanoseconds(103203758));
			BOOST_CHECK_EQUAL(timedLog->getLastCommandTimeAs<std::chrono::milliseconds>().count(), 103);
		}

		for(std::size_t i = 0; i < 6; i++)
		{
			log->read();
		}

		BOOST_CHECK(log->isEnd());
		BOOST_CHECK(bool(log->getStream()) == !log->isEnd());
	}
}

BOOST_FIXTURE_TEST_CASE(LogReadStringName, LogFixture)
{
	std::string logCommand;
	std::string response = stringNameLog->read(logCommand);

	BOOST_CHECK_EQUAL(logCommand, "getscan\n");
	BOOST_CHECK_EQUAL(response, "2\n");

	const regilo::LogMetadata *metadata = stringNameLog->getMetadata();
	BOOST_REQUIRE(metadata != nullptr);

	BOOST_CHECK_EQUAL(metadata->getType(), "stringlog");
	BOOST_CHECK_EQUAL(metadata->getVersion(), 2);
}

BOOST_FIXTURE_TEST_CASE(LogSkipComments, LogFixture)
{
	std::string logCommand;
	std::string response = commentLog->read(logCommand);

	BOOST_CHECK_EQUAL(logCommand, "V\n");
	BOOST_CHECK_EQUAL(response, "2\n");

	const regilo::LogMetadata *metadata = commentLog->getMetadata();
	BOOST_REQUIRE(metadata != nullptr);

	BOOST_CHECK_EQUAL(metadata->getType(), "log");
	BOOST_CHECK_EQUAL(metadata->getVersion(), 2);
}

BOOST_AUTO_TEST_CASE(LogWrite)
{
	std::string contents[] = {
		"type log\nversion 2\n\nc 4 cmd1\nr 9 response1\n\nc 4 cmd2\nr 9 response2\n\n",
		"type timedlog\nversion 2\ntimeres 1 1\n\nc 4 cmd1\nr 9 response1\nt 0\n\nc 4 cmd2\nr 9 response2\nt 0\n\n"
	};
	regilo::ILog *logs[] = { new regilo::Log("log.txt"), new regilo::TimedLog<std::chrono::seconds>("timed-log.txt") };

	for(std::size_t i = 0; i < 2; i++)
	{
		regilo::ILog *log = logs[i];

		BOOST_CHECK(log->getStream());
		if(log->getStream())
		{
			log->write("cmd1", "response1");
			log->write("cmd2", "response2");
		}
		log->close();

		std::ifstream logFile(log->getFilePath());
		std::string content;
		std::getline(logFile, content, '\0');
		BOOST_CHECK_EQUAL(content, contents[i]);

		std::remove(log->getFilePath().c_str());
		delete log;
	}
}

BOOST_FIXTURE_TEST_CASE(LogReadCommand, LogFixture)
{
	for(std::size_t i = 1; i < logs.size(); i += 2)
	{
		regilo::ILog *streamLog = logs.at(i);

		std::string response1 = streamLog->readCommand("V");
		BOOST_CHECK_EQUAL(response1, "0\nVERSION1");

		std::string logCommand;
		std::string response2 = streamLog->readCommand("V", logCommand);
		BOOST_CHECK_EQUAL(logCommand, "V\n");
		BOOST_CHECK_EQUAL(response2, "0\nVERSION2");
	}
}

BOOST_FIXTURE_TEST_CASE(TimedLogNonSyncRead, LogFixture)
{
	BOOST_REQUIRE(timedFileLog->getStream());

	auto start = std::chrono::high_resolution_clock::now();

	std::string logCommand;
	while(!timedFileLog->isEnd())
	{
		timedFileLog->read(logCommand);
	}

	std::chrono::nanoseconds elapsed = std::chrono::high_resolution_clock::now() - start;
	std::chrono::nanoseconds lastCommandNanoseconds = timedFileLog->getLastCommandNanoseconds();

	BOOST_CHECK(lastCommandNanoseconds > elapsed);
}

BOOST_FIXTURE_TEST_CASE(TimedLogSyncRead, LogFixture)
{
	BOOST_REQUIRE(timedFileLog->getStream());
	timedFileLog->syncTime();

	auto start = std::chrono::high_resolution_clock::now();

	std::string logCommand;
	while(!timedFileLog->isEnd())
	{
		timedFileLog->read(logCommand);
	}

	std::chrono::nanoseconds elapsed = std::chrono::high_resolution_clock::now() - start;
	std::chrono::nanoseconds lastCommandNanoseconds = timedFileLog->getLastCommandNanoseconds();

	BOOST_CHECK(lastCommandNanoseconds < elapsed);
}

BOOST_AUTO_TEST_SUITE_END()
