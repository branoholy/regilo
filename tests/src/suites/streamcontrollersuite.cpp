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
#include <mutex>
#include <sstream>
#include <thread>
#include <type_traits>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <boost/mpl/vector.hpp>

#include "regilo/serialcontroller.hpp"
#include "regilo/socketcontroller.hpp"

#include "simulators/serialsimulator.hpp"
#include "simulators/socketsimulator.hpp"

#define SF StreamControllerFixture<StreamController>

template<typename StreamController>
struct StreamControllerFixture
{
	std::string logPath = "data/hokuyo-log.txt";
	std::stringstream logStream;

	std::vector<StreamController*> controllers;

	StreamControllerFixture() :
		logStream("1$CMD1\n$RESPONSE1$V\n$RESPONSE2$CMD3\n$2.5$CMD 4 5\n$RESPONSE4$CMD6\n$5$")
	{
		controllers.push_back(new StreamController(logPath));
		controllers.push_back(new StreamController(logStream));
	}

	inline const StreamController* getFileController() const { return controllers.at(0); }
};

typedef boost::mpl::vector<regilo::SerialController, regilo::SocketController> StreamControllers;

BOOST_AUTO_TEST_SUITE(StreamControllerSuite)

BOOST_FIXTURE_TEST_CASE_TEMPLATE(StreamControllerConstructorValues, StreamController, StreamControllers, SF)
{
	BOOST_CHECK(SF::controllers.at(0)->getLog()->getFilePath() == SF::logPath);
	BOOST_CHECK(&(SF::controllers.at(1)->getLog()->getStream()) == &(SF::logStream));

	const StreamController *constController = SF::getFileController();
	BOOST_CHECK(constController->getLog()->getFilePath() == SF::logPath);
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(StreamControllerLog, StreamController, StreamControllers, SF)
{
	auto log = std::make_shared<regilo::Log>(SF::logPath);
	SF::controllers.at(0)->setLog(log);

	BOOST_CHECK(SF::controllers.at(0)->getLog()->getFilePath() == SF::logPath);
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(StreamControllerCommunication, StreamController, StreamControllers, SF)
{
	std::mutex mutex;
	mutex.lock();

	std::string deviceEndpoint;
	bool deviceStatus = false;
	std::thread deviceThread([this, &deviceEndpoint, &mutex, &deviceStatus]()
	{
		Simulator *simulator = nullptr;
		if(std::is_same<StreamController, regilo::SerialController>::value)
		{
			simulator = new SerialSimulator(SF::logStream);
		}
		else if(std::is_same<StreamController, regilo::SocketController>::value)
		{
			simulator = new SocketSimulator(SF::logStream, 12345);
		}

		BOOST_REQUIRE(simulator != nullptr);

		simulator->start();
		deviceEndpoint = simulator->getEndpoint();
		mutex.unlock();

		deviceStatus = simulator->run();
		mutex.lock();

		delete simulator;
	});

	StreamController controller;

	BOOST_CHECK(!controller.isConnected());
	BOOST_CHECK(controller.getEndpoint().empty());

	mutex.lock();
	controller.connect(deviceEndpoint);

	BOOST_REQUIRE(controller.isConnected());
	BOOST_CHECK_EQUAL(controller.getEndpoint(), deviceEndpoint);

	std::string response1 = ((regilo::IController*)&controller)->sendCommand("CMD1");
	BOOST_CHECK_EQUAL(response1, "RESPONSE1");

	std::string response2 = controller.template sendCommand<std::string>('V');
	BOOST_CHECK_EQUAL(response2, "RESPONSE2");

	double response3 = controller.template sendCommand<double>("CMD3");
	BOOST_CHECK_CLOSE(response3, 2.5, 0.00001);

	std::string response4 = controller.template sendCommand<std::string>("CMD", 4, 5);
	BOOST_CHECK_EQUAL(response4, "RESPONSE4");

	int response5 = controller.template sendFormattedCommand<int>("CMD%d", 6);
	BOOST_CHECK_EQUAL(response5, 5);

	mutex.unlock();

	if(deviceThread.joinable()) deviceThread.join();

	BOOST_CHECK(deviceStatus);
}

BOOST_AUTO_TEST_SUITE_END()
