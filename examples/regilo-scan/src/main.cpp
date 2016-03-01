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
#include <thread>

#include <regilo/neatocontroller.hpp>
#include <regilo/hokuyocontroller.hpp>

#include <regilo/serialcontroller.hpp>
#include <regilo/socketcontroller.hpp>

#include <regilo/version.hpp>

void printHelp()
{
	std::cout << "Usage: regilo-scan [options]" << std::endl
			  << "Options:" << std::endl
			  << "  -c <controller>    The controller name in format \"device:protocol\"." << std::endl
			  << "                     Device can be \"neato\" or \"hokuyo\" (default: \"neato\")." << std::endl
			  << "                     Protocol can be \"socket\" or \"serial\" (default: \"socket\")." << std::endl
			  << "  -e <endpoint>      The endpoint that is used to connect to the device" << std::endl
			  << "                     (path or ip and port, default: \"10.0.0.1:12345\")." << std::endl
			  << "  -h                 Show this help." << std::endl
			  << std::endl
			  << "Using regilo-" << regilo::Version::VERSION << std::endl;
}

int main(int argc, char** argv)
{
	std::cout.setf(std::ios_base::boolalpha);

	std::string controllerDevice = "neato";
	std::string controllerProtocol = "socket";
	std::string controllerName = controllerDevice + ':' + controllerProtocol;

	std::string endpoint = "10.0.0.1:12345";
	for(int i = 1; i < argc; i++)
	{
		std::string arg(argv[i]);
		if(arg == "-c")
		{
			controllerName = std::string(argv[++i]);

			std::size_t colonPos = controllerName.find(':');
			if(colonPos == std::string::npos)
			{
				std::cout << "Error: Missing controller protocol (use \"socket\" or \"serial\")." << std::endl;
				return 1;
			}

			controllerDevice = controllerName.substr(0, colonPos);
			if(controllerDevice != "neato" && controllerDevice != "hokuyo")
			{
				std::cout << "Error: Unknown controller device (use \"neato\" or \"hokuyo\")." << std::endl;
				return 1;
			}

			controllerProtocol = controllerName.substr(colonPos + 1);
			if(controllerProtocol != "socket" && controllerProtocol != "serial")
			{
				std::cout << "Error: Unknown controller protocol (use \"socket\" or \"serial\")." << std::endl;
				return 1;
			}
		}
		else if(arg == "-e") endpoint = std::string(argv[++i]);
		else
		{
			printHelp();
			return 0;
		}
	}

	std::cout << "Hello Regilo!" << std::endl;

	regilo::IScanController *controller = nullptr;
	regilo::INeatoController *neatoController = nullptr;
	regilo::IHokuyoController *hokuyoController = nullptr;

	if(controllerName == "neato:socket") controller = dynamic_cast<regilo::IScanController*>(neatoController = new regilo::NeatoSocketController());
	else if(controllerName == "neato:serial") controller = dynamic_cast<regilo::IScanController*>(neatoController = new regilo::NeatoSerialController());
	else if(controllerName == "hokuyo:socket") controller = dynamic_cast<regilo::IScanController*>(hokuyoController = new regilo::HokuyoSocketController());
	else if(controllerName == "hokuyo:serial") controller = dynamic_cast<regilo::IScanController*>(hokuyoController = new regilo::HokuyoSerialController());

	std::cout << "Using " << controllerName << " controller." << std::endl;

	std::cout << "Connecting to " << endpoint << std::endl;
	controller->connect(endpoint);

	if(controllerDevice == "neato")
	{
		neatoController->setTestMode(true);
		std::cout << "Test mode: " << neatoController->getTestMode() << std::endl;

		neatoController->setLdsRotation(true);
		std::cout << "LDS rotation: " << neatoController->getLdsRotation() << std::endl;
	}

	std::this_thread::sleep_for(std::chrono::seconds(3));

	regilo::ScanData data = controller->getScan();
	std::cout << "Scan data:" << std::endl << data << std::endl;

	if(controllerDevice == "neato")
	{
		neatoController->setLdsRotation(false);
		std::cout << "LDS rotation: " << neatoController->getLdsRotation() << std::endl;

		neatoController->setTestMode(false);
		std::cout << "Test mode: " << neatoController->getTestMode() << std::endl;
	}
	else if(controllerDevice == "hokuyo")
	{
		std::map<std::string, std::string> info = hokuyoController->getVersionInfo();
		std::cout << "Version info: " << std::endl;
		for(const auto& kw : info)
		{
			std::cout << kw.first << " = " << kw.second << std::endl;
		}
		std::cout << std::endl;
	}

	delete controller;

	return 0;
}
