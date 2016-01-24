/*
 * NeatoC
 * Copyright (C) 2015-2016  Branislav Holý <branoholy@gmail.com>
 *
 * This file is part of NeatoC.
 *
 * NeatoC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NeatoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NeatoC.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>

#include <neatoc/neatocontroller.hpp>
#include <neatoc/hokuyocontroller.hpp>

#include "neatocscanapp.hpp"

void printHelp()
{
	std::cout << "Usage: neatoc-scan-gui [options]" << std::endl
			  << "Options:" << std::endl
			  << "  -c <controller>    The controller name (\"neato\" or \"hokuyo\", default: \"neato\")." << std::endl
			  << "  -e <endpoint>      The endpoint that is used to connect to the device" << std::endl
			  << "                     (path or ip and port, default: \"10.0.0.1:12345\")." << std::endl
			  << "                     Use string \"log\" to load a log file." << std::endl
			  << "  -l <file>          The path to the log file." << std::endl
			  << "  -m                 Turn on manual scanning (by pressing key S)." << std::endl
			  << "  -a                 Turn on automatic scanning before move." << std::endl
			  << "  -h                 Show this help." << std::endl;
}

int main(int argc, char** argv)
{
	std::cout.setf(std::ios_base::boolalpha);

	std::string controllerName = "neato";
	std::string endpoint = "10.0.0.1:12345";
	std::string logPath;
	bool manualScanning = false;
	bool moveScanning = false;
	for(int i = 1; i < argc; i++)
	{
		std::string arg(argv[i]);
		if(arg == "-c")
		{
			controllerName = std::string(argv[++i]);

			if(controllerName != "neato" && controllerName != "hokuyo")
			{
				std::cout << "Error: Unknown controller (use \"neato\" or \"hokuyo\")." << std::endl;
				return 1;
			}
		}
		else if(arg == "-e") endpoint = std::string(argv[++i]);
		else if(arg == "-l") logPath = std::string(argv[++i]);
		else if(arg == "-m") manualScanning = true;
		else if(arg == "-a") moveScanning = true;
		else
		{
			printHelp();
			return 0;
		}
	}
	bool useScanner = (endpoint != "log");

	std::cout << "Hello NeatoC!" << std::endl;

	neatoc::Controller *controller;
	if(controllerName == "neato") controller = new neatoc::NeatoController(logPath);
	else controller = new neatoc::HokuyoController(logPath);

	std::cout << "Using " << controllerName << " controller." << std::endl;

	std::cout << "Connecting to " << endpoint << std::endl;
	if(useScanner)
	{
		controller->connect(endpoint);

		if(controllerName == "neato")
		{
			neatoc::NeatoController *neatoController = static_cast<neatoc::NeatoController*>(controller);

			neatoController->setTestMode(true);
			std::cout << "Test mode: " << neatoController->getTestMode() << std::endl;

			neatoController->setLdsRotation(true);
			std::cout << "LDS rotation: " << neatoController->getLdsRotation() << std::endl;
		}
	}

	NeatocScanApp *app = new NeatocScanApp(controller, useScanner, manualScanning, moveScanning);
	NeatocScanApp::Display(app, argc, argv);

	if(useScanner && controllerName == "neato")
	{
		neatoc::NeatoController *neatoController = static_cast<neatoc::NeatoController*>(controller);

		neatoController->setLdsRotation(false);
		std::cout << "LDS rotation: " << neatoController->getLdsRotation() << std::endl;

		neatoController->setTestMode(false);
		std::cout << "Test mode: " << neatoController->getTestMode() << std::endl;
	}

	delete controller;

	return 0;
}
