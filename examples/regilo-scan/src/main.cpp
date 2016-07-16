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

#include <regilo/hokuyocontroller.hpp>
#include <regilo/neatocontroller.hpp>

#include <regilo/serialcontroller.hpp>
#include <regilo/socketcontroller.hpp>

#include <regilo/version.hpp>

struct Arguments
{
	std::string device;
	std::string protocol;
	std::string endpoint;
	std::string logPath;
	bool help = false;
};

void printHelp();
void parseArgs(int argc, char **argv, Arguments& args);

int main(int argc, char **argv)
{
	std::cout.setf(std::ios_base::boolalpha);

	Arguments args;
	try
	{
		parseArgs(argc, argv, args);
	}
	catch(std::invalid_argument& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return 1;
	}

	if(args.help)
	{
		printHelp();
		return 0;
	}

	std::cout << "Hello Regilo!" << std::endl;

	bool fromDevice = (args.protocol != "log");
	regilo::IScanController *controller = nullptr;
	regilo::INeatoController *neatoController = nullptr;
	regilo::IHokuyoController *hokuyoController = nullptr;

	if(args.device == "neato")
	{
		if(args.protocol == "serial") neatoController = new regilo::NeatoSerialController();
		else neatoController = new regilo::NeatoSocketController();

		controller = dynamic_cast<regilo::IScanController*>(neatoController);
	}
	else if(args.device == "hokuyo")
	{
		if(args.protocol == "serial") hokuyoController = new regilo::HokuyoSerialController();
		else hokuyoController = new regilo::HokuyoSocketController();

		controller = dynamic_cast<regilo::IScanController*>(hokuyoController);
	}

	std::cout << "Using " << args.device << ':' << args.protocol << " controller." << std::endl;
	std::cout << "Connecting to " << args.endpoint << std::endl;

	regilo::TimedLog<> *log;
	if(fromDevice)
	{
		controller->connect(args.endpoint);
		if(!args.logPath.empty()) log = new regilo::TimedLog<>(args.logPath);
	}
	else log = new regilo::TimedLog<>(args.endpoint);
	controller->setLog(std::shared_ptr<regilo::TimedLog<>>(log));

	if(fromDevice && args.device == "neato")
	{
		neatoController->setTestMode(true);
		std::cout << "Test mode: " << neatoController->getTestMode() << std::endl;

		neatoController->setLdsRotation(true);
		std::cout << "LDS rotation: " << neatoController->getLdsRotation() << std::endl;
	}

	std::this_thread::sleep_for(std::chrono::seconds(3));

	regilo::ScanData data = controller->getScan(fromDevice);
	std::cout << "Scan data:" << std::endl << data << std::endl;

	if(fromDevice)
	{
		if(args.device == "neato")
		{
			neatoController->setLdsRotation(false);
			std::cout << "LDS rotation: " << neatoController->getLdsRotation() << std::endl;

			neatoController->setTestMode(false);
			std::cout << "Test mode: " << neatoController->getTestMode() << std::endl;
		}
		else if(args.device == "hokuyo")
		{
			std::map<std::string, std::string> info = hokuyoController->getVersionInfo();
			std::cout << "Version info: " << std::endl;
			for(const auto& kw : info)
			{
				std::cout << kw.first << " = " << kw.second << std::endl;
			}
			std::cout << std::endl;
		}
	}

	delete controller;

	return 0;
}

void printHelp()
{
	std::cout << "Usage: regilo-scan [options] <controller> <endpoint>" << std::endl
			  << "Arguments:" << std::endl
			  << "  <controller>  The controller name in the format \"device:protocol\". The device" << std::endl
			  << "                part can be \"neato\" or \"hokuyo\". The protocol part can be" << std::endl
			  << "                \"socket\", \"serial\", or \"log\"." << std::endl
			  << "  <endpoint>    The endpoint that is used to connect to the device. It can be" << std::endl
			  << "                a path to the device or input log, or ip and port." << std::endl
			  << std::endl
			  << "Options:" << std::endl
			  << "  -l <file>     The path to the output log file." << std::endl
			  << "  -h, --help    Show this help." << std::endl
			  << std::endl
			  << "Using regilo-" << regilo::Version::VERSION << std::endl;
}

void parseArgs(int argc, char **argv, Arguments& args)
{
	int posArg = 0;
	for(int i = 1; i < argc; i++)
	{
		std::string arg(argv[i]);

		if(arg == "-l") args.logPath = std::string(argv[++i]);
		else if(arg == "-h" || arg == "--help")
		{
			args.help = true;
			return;
		}
		else if(arg.front() != '-')
		{
			if(posArg == 0)
			{
				std::size_t colonPos = arg.find(':');
				if(colonPos == std::string::npos)
					throw std::invalid_argument("Missing controller protocol (use \"socket\", \"serial\", or \"log\").");

				args.device = arg.substr(0, colonPos);
				if(args.device != "neato" && args.device != "hokuyo")
					throw std::invalid_argument("Unknown controller device (use \"neato\" or \"hokuyo\").");

				args.protocol = arg.substr(colonPos + 1);
				if(args.protocol != "socket" && args.protocol != "serial" && args.protocol != "log")
					throw std::invalid_argument("Unknown controller protocol (use \"socket\", \"serial\", or \"log\").");
			}
			else if(posArg == 1) args.endpoint = arg;

			posArg++;
		}
		else throw std::invalid_argument("Unknown argument \"" + arg + "\".");
	}

	if(args.device.empty() || args.protocol.empty())
		throw std::invalid_argument("Missing controller (see -h for more details).");

	if(args.endpoint.empty())
		throw std::invalid_argument("Missing endpoint (see -h for more details).");
}
