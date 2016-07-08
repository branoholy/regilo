#include <chrono>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

#include "regilo/utils.hpp"

BOOST_AUTO_TEST_SUITE(UtilsSuite)

typedef boost::mpl::list<std::chrono::nanoseconds, std::chrono::milliseconds, std::chrono::seconds> TimeTypes;

BOOST_AUTO_TEST_CASE_TEMPLATE(TimeSinceEpoch, T, TimeTypes)
{
	auto epochTimeBefore = std::chrono::system_clock::now().time_since_epoch();
	T timeBefore = std::chrono::duration_cast<T>(epochTimeBefore);

	T time = regilo::epoch<T>();

	auto epochTimeAfter = std::chrono::system_clock::now().time_since_epoch();
	T timeAfter = std::chrono::duration_cast<T>(epochTimeAfter);

	BOOST_REQUIRE(timeBefore <= time);
	BOOST_REQUIRE(timeAfter >= time);
}

BOOST_AUTO_TEST_SUITE_END()
