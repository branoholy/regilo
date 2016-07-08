#include <iostream>
#include <sstream>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "regilo/log.hpp"

struct TimedLogFixture
{
	std::string logPath = "data/hokuyo-timed-log.txt";
	std::stringstream logStream;

	std::vector<regilo::TimedLog<std::chrono::nanoseconds>*> timedLogs;

	TimedLogFixture() :
		logStream("1 1 1000000000$G00076801\n$0\n0C0C0C0C0C0C0C0C0C0C$103203758$")
	{
		timedLogs.push_back(new regilo::TimedLog<std::chrono::nanoseconds>(logPath));
		timedLogs.emplace_back(new regilo::TimedLog<std::chrono::nanoseconds>(logStream));
	}

	~TimedLogFixture()
	{
		for(regilo::TimedLog<std::chrono::nanoseconds> *log : timedLogs)
		{
			delete log;
		}
	}
};

BOOST_AUTO_TEST_SUITE(LogSuite)

BOOST_FIXTURE_TEST_CASE(LogConstructorValues, TimedLogFixture)
{
	BOOST_CHECK(timedLogs.at(0)->getFilePath() == logPath);
	BOOST_CHECK(&timedLogs.at(1)->getStream() == &logStream);
}

BOOST_FIXTURE_TEST_CASE(FileLogRead, TimedLogFixture)
{
	for(regilo::TimedLog<std::chrono::nanoseconds> *log : timedLogs)
	{
		BOOST_REQUIRE(log->getStream());
		BOOST_CHECK_EQUAL(bool(log->getStream()), !log->isEnd());

		std::string logCommand;
		std::string logResponse = log->read(logCommand);

		BOOST_CHECK_EQUAL(logCommand, "G00076801\n");

		BOOST_CHECK_EQUAL(logResponse.substr(0, 22), "0\n0C0C0C0C0C0C0C0C0C0C");

		BOOST_CHECK(log->getLastCommandNanoseconds() == std::chrono::nanoseconds(103203758));
		BOOST_CHECK(log->getLastCommandTime() == std::chrono::nanoseconds(103203758));
		BOOST_CHECK_EQUAL(log->getLastCommandTimeAs<std::chrono::milliseconds>().count(), 103);

		for(std::size_t i = 0; i < 6; i++)
		{
			log->read(logCommand);
		}

		BOOST_CHECK(log->isEnd());
		BOOST_CHECK(bool(log->getStream()) == !log->isEnd());
	}
}

BOOST_AUTO_TEST_SUITE_END()
