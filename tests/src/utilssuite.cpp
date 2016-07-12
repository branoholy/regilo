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
