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

#include "regilo/hokuyocontroller.hpp"

#include "simulators/serialsimulator.hpp"
#include "simulators/socketsimulator.hpp"

#define HF HokuyoControllerFixture<HokuyoController>

template<typename HokuyoController>
struct HokuyoControllerFixture
{
	std::string logPath = "data/hokuyo-log-scan-version.txt";
	std::stringstream logStream;

	std::vector<HokuyoController*> controllers;

	std::string correctScan;
	std::map<std::string, std::string> correctVersion;

	HokuyoControllerFixture() :
		logStream("1$CMD1\n$RESPONSE1$")
	{
		controllers.emplace_back(new HokuyoController());
		controllers.emplace_back(new HokuyoController(logPath));
		controllers.emplace_back(new HokuyoController(logStream));

		std::ifstream dataFile("data/hokuyo-correct-scan.txt");
		std::getline(dataFile, correctScan, '\0');

		correctVersion["VEND"] = "Hokuyo Automatic Co.,Ltd.";
		correctVersion["PROD"] = "SOKUIKI Sensor URG-04LX";
		correctVersion["FIRM"] = "3.3.00,08/04/16(20-4095[mm],240[deg],44-725[step],600[rpm])";
		correctVersion["PROT"] = "00003,(SCIP 1.0)";
		correctVersion["SERI"] = "H0713090";
		correctVersion["STAT"] = "FW Normal[FinalDist with shadow  ]";
	}

	~HokuyoControllerFixture()
	{
		for(HokuyoController *controller : controllers)
		{
			delete controller;
		}
	}
};

typedef boost::mpl::vector<regilo::HokuyoSerialController, regilo::HokuyoSocketController> HokuyoControllers;

BOOST_AUTO_TEST_SUITE(HokuyoControllerSuite)

BOOST_FIXTURE_TEST_CASE_TEMPLATE(HokuyoControllerConstructorValues, HokuyoController, HokuyoControllers, HF)
{
	for(HokuyoController *controller : HF::controllers)
	{
		BOOST_CHECK_EQUAL(controller->RESPONSE_END, "\n\n");
	}
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(HokuyoControllerScanFromDevice, HokuyoController, HokuyoControllers, HF)
{
	std::mutex mutex;
	mutex.lock();

	std::string deviceEndpoint;
	bool deviceStatus = false;
	std::thread deviceThread([this, &deviceEndpoint, &mutex, &deviceStatus]()
	{
		Simulator *simulator = nullptr;
		if(std::is_same<HokuyoController, regilo::HokuyoSerialController>::value)
		{
			simulator = new SerialSimulator(HF::logPath);
		}
		else if(std::is_same<HokuyoController, regilo::HokuyoSocketController>::value)
		{
			simulator = new SocketSimulator(HF::logPath, 12345);
		}

		simulator->responseEnd = "\n\n";

		BOOST_REQUIRE(simulator != nullptr);

		simulator->start();
		deviceEndpoint = simulator->getEndpoint();
		mutex.unlock();

		deviceStatus = simulator->run();
		mutex.lock();

		delete simulator;
	});

	HokuyoController *controller = HF::controllers.at(0);

	mutex.lock();
	controller->connect(deviceEndpoint);

	BOOST_REQUIRE(controller->isConnected());

	regilo::ScanData scandata = controller->getScan();
	std::ostringstream scanStream;
	scanStream << scandata;

	BOOST_CHECK_EQUAL(scanStream.str(), HF::correctScan);

	std::map<std::string, std::string> version = controller->getVersionInfo();
	BOOST_CHECK(version == HF::correctVersion);

	mutex.unlock();

	if(deviceThread.joinable()) deviceThread.join();

	BOOST_CHECK(deviceStatus);
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(HokuyoControllerScanFromLog, HokuyoController, HokuyoControllers, HF)
{
	HokuyoController *controller = HF::controllers.at(1);

	regilo::ScanData scanData = controller->getScan(false);
	std::ostringstream scanStream;
	scanStream << scanData;

	BOOST_CHECK_EQUAL(scanStream.str(), HF::correctScan);
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(HokuyoControllerSetScanParameters, HokuyoController, HokuyoControllers, HF)
{
	HokuyoController *controller = HF::controllers.at(0);

	BOOST_CHECK_NO_THROW(controller->setScanParameters(50, 80, 1));
	BOOST_CHECK_THROW(controller->setScanParameters(800, 900, 1), std::invalid_argument);
	BOOST_CHECK_THROW(controller->setScanParameters(50, 800, 1), std::invalid_argument);
	BOOST_CHECK_THROW(controller->setScanParameters(50, 80, 100), std::invalid_argument);
	BOOST_CHECK_THROW(controller->setScanParameters(80, 50, 1), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()
