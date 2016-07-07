#include <chrono>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include "regilo/utils.hpp"

BOOST_AUTO_TEST_SUITE(UtilsSuite)

BOOST_AUTO_TEST_CASE(TimeSinceEpoch)
{
	auto epochSecondsBefore = std::chrono::system_clock::now().time_since_epoch();
	std::chrono::seconds secondsBefore = std::chrono::duration_cast<std::chrono::seconds>(epochSecondsBefore);

	std::chrono::seconds seconds = regilo::epoch<std::chrono::seconds>();

	auto epochSecondsAfter = std::chrono::system_clock::now().time_since_epoch();
	std::chrono::seconds secondsAfter = std::chrono::duration_cast<std::chrono::seconds>(epochSecondsAfter);

	BOOST_REQUIRE(secondsBefore <= seconds);
	BOOST_REQUIRE(secondsAfter >= seconds);
}

BOOST_AUTO_TEST_SUITE_END()
