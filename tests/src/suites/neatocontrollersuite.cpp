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

#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test.hpp>

#include "regilo/neatocontroller.hpp"

#include "simulators/serialsimulator.hpp"
#include "simulators/socketsimulator.hpp"

#define NF NeatoControllerFixture<NeatoController>

template<typename NeatoController>
struct NeatoControllerFixture
{
    std::string logPath = "data/neato-log-scan-move-time.txt";
    std::string timedLogPath = "data/neato-timed-log-scan-move-time.txt";
    std::stringstream logStream;

    std::vector<NeatoController*> controllers;

    std::string correctScan;
    std::string correctTime = "Sunday 13:57:09";

    NeatoControllerFixture() :
        logStream("type log\nversion 2\n\nc 11 getldsscan\n\nr 13 AngleInDegre\n\n\n")
    {
        controllers.emplace_back(new NeatoController());
        controllers.emplace_back(new NeatoController(logPath));
        controllers.emplace_back(new NeatoController(logStream));

        std::ifstream dataFile("data/neato-correct-scan.txt");
        std::getline(dataFile, correctScan, '\0');
    }

    ~NeatoControllerFixture()
    {
        for(NeatoController *controller : controllers)
        {
            delete controller;
        }
    }
};

typedef boost::mpl::vector<regilo::NeatoSerialController, regilo::NeatoSocketController> NeatoControllers;

BOOST_AUTO_TEST_SUITE(NeatoControllerSuite)

BOOST_FIXTURE_TEST_CASE_TEMPLATE(NeatoControllerConstructorValues, NeatoController, NeatoControllers, NF)
{
    for(NeatoController *controller : NF::controllers)
    {
        BOOST_CHECK_EQUAL(controller->RESPONSE_END, std::string(1, 0x1a));
    }
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(NeatoControllerScanFromDevice, NeatoController, NeatoControllers, NF)
{
    std::mutex mutex;
    mutex.lock();

    std::string deviceEndpoint;
    bool deviceStatus = false;
    std::thread deviceThread([this, &deviceEndpoint, &mutex, &deviceStatus] ()
    {
        Simulator *simulator = nullptr;
        if(std::is_same<NeatoController, regilo::NeatoSerialController>::value)
        {
            simulator = new SerialSimulator(NF::logPath);
        }
        else if(std::is_same<NeatoController, regilo::NeatoSocketController>::value)
        {
            simulator = new SocketSimulator(NF::logPath, 12345);
        }

        simulator->responseEnd = std::string(1, 0x1a);

        BOOST_REQUIRE(simulator != nullptr);

        simulator->start();
        deviceEndpoint = simulator->getEndpoint();
        mutex.unlock();

        deviceStatus = simulator->run();
        mutex.lock();

        delete simulator;
    });

    NeatoController *controller = NF::controllers.at(0);

    mutex.lock();
    controller->connect(deviceEndpoint);

    BOOST_REQUIRE(controller->isConnected());

    BOOST_CHECK(!controller->getTestMode());
    BOOST_CHECK(!controller->getLdsRotation());

    controller->startScanner();

    BOOST_REQUIRE(controller->getTestMode());
    BOOST_REQUIRE(controller->getLdsRotation());

    regilo::ScanData scanData = controller->getScan();
    std::ostringstream scanStream;
    scanStream << scanData;

    BOOST_CHECK_EQUAL(scanStream.str(), NF::correctScan);

    controller->setMotor(100, 100, 50);
    std::string time = controller->getTime();
    BOOST_CHECK_EQUAL(time, NF::correctTime);

    controller->stopScanner();

    BOOST_CHECK(!controller->getTestMode());
    BOOST_CHECK(!controller->getLdsRotation());

    mutex.unlock();

    if(deviceThread.joinable()) deviceThread.join();

    BOOST_CHECK(deviceStatus);
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(NeatoControllerScanFromLog, NeatoController, NeatoControllers, NF)
{
    NeatoController *controller = NF::controllers.at(1);

    regilo::ScanData scanData = controller->getScan(false);
    std::ostringstream scanStream;
    scanStream << scanData;

    BOOST_CHECK_EQUAL(scanStream.str(), NF::correctScan);
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(NeatoControllerScanFromTimedLog, NeatoController, NeatoControllers, NF)
{
    NeatoController *controller = NF::controllers.at(0);
    auto timedLog = std::make_shared<regilo::TimedLog<>>(NF::timedLogPath);
    controller->setLog(timedLog);

    regilo::ScanData scanData = controller->getScan(false);
    std::ostringstream scanStream;
    scanStream << scanData;

    BOOST_CHECK_EQUAL(scanStream.str(), NF::correctScan);
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(NeatoControllerWrongHeader, NeatoController, NeatoControllers, NF)
{
    NeatoController *controller = NF::controllers.at(2);

    regilo::ScanData scanData = controller->getScan(false);
    BOOST_CHECK(scanData.empty());
}

BOOST_AUTO_TEST_SUITE_END()
