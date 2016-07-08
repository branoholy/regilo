#include <cstdio>
#include <iostream>
#include <sstream>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "regilo/log.hpp"

struct LogFixture
{
	std::string logPath = "data/hokuyo-log.txt";
	std::string timedLogPath = "data/hokuyo-timed-log.txt";
	std::stringstream logStream;
	std::stringstream timedLogStream;

	regilo::Log *fileLog;
	regilo::Log *streamLog;

	regilo::TimedLog<std::chrono::nanoseconds> *timedFileLog;
	regilo::TimedLog<std::chrono::nanoseconds> *timedStreamLog;

	std::vector<regilo::ILog*> logs;

	LogFixture() :
		logStream("1$G00076801\n$0\n0C0C0C0C0C0C0C0C0C0C$V\n$0\nVERSION1$G00076801\n$0\n0C0C0C0C0C0C0C0C0C0C$V\n$0\nVERSION2$"),
		timedLogStream("1 1 1000000000$G00076801\n$0\n0C0C0C0C0C0C0C0C0C0C$103203758$V\n$0\nVERSION1$103203759$G00076801\n$0\n0C0C0C0C0C0C0C0C0C0C$103203760$V\n$0\nVERSION2$103203761$"),
		fileLog(new regilo::Log(logPath)),
		streamLog(new regilo::Log(logStream)),
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
	BOOST_CHECK(logs.at(0)->getFilePath() == logPath);
	BOOST_CHECK(&logs.at(1)->getStream() == &logStream);
	BOOST_CHECK(logs.at(2)->getFilePath() == timedLogPath);
	BOOST_CHECK(&logs.at(3)->getStream() == &timedLogStream);
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

BOOST_AUTO_TEST_CASE(LogWrite)
{
	std::string contents[] = {"1$cmd1$response1$cmd2$response2$", "1 1 1000$cmd1$response1$0$cmd2$response2$0$"};
	regilo::ILog *logs[] = {new regilo::Log("log.txt"), new regilo::TimedLog<std::chrono::milliseconds>("timed-log.txt")};

	for(std::size_t i = 0; i < 2; i++)
	{
		regilo::ILog *log = logs[i];
		std::string path = log->getFilePath();

		BOOST_CHECK(log->getStream());
		if(log->getStream())
		{
			log->write("cmd1", "response1");
			log->write("cmd2", "response2");
		}

		delete log;

		std::ifstream logFile(path);
		std::string line;
		std::getline(logFile, line);
		BOOST_CHECK_EQUAL(line, contents[i]);

		std::remove(path.c_str());
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
