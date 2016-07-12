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

#include <iostream>
#include <sstream>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "regilo/serialcontroller.hpp"

struct SerialControllerFixture
{
	std::string logPath = "data/hokuyo-log.txt";
	std::stringstream logStream;

	std::vector<regilo::SerialController*> controllers;

	SerialControllerFixture() :
		logStream("1$G00076801\n$0\n0C0C0C0C0C0C0C0C0C0C$V\n$0\nVERSION1$G00076801\n$0\n0C0C0C0C0C0C0C0C0C0C$V\n$0\nVERSION2$")
	{
		controllers.push_back(new regilo::SerialController(logPath));
		controllers.push_back(new regilo::SerialController(logStream));
	}

	~SerialControllerFixture()
	{
		for(regilo::SerialController *controller : controllers)
		{
			delete controller;
		}
	}
};

BOOST_AUTO_TEST_SUITE(SerialControllerSuite)

BOOST_FIXTURE_TEST_CASE(SerialControllerConstructorValues, SerialControllerFixture)
{
	BOOST_CHECK(controllers.at(0)->getLog()->getFilePath() == logPath);
	BOOST_CHECK(&controllers.at(1)->getLog()->getStream() == &logStream);
}

BOOST_FIXTURE_TEST_CASE(SerialControllerLog, SerialControllerFixture)
{
	auto log = std::make_shared<regilo::Log>(logPath);
	controllers.at(0)->setLog(log);

	BOOST_CHECK(controllers.at(0)->getLog()->getFilePath() == logPath);
}

BOOST_AUTO_TEST_SUITE_END()
