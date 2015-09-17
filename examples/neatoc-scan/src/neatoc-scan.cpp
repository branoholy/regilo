/*
 * NeatoC
 * Copyright (C) 2015  Branislav Holý <branoholy@gmail.com>
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

#include <chrono>
#include <iostream>
#include <thread>

#include <neatoc/controller.hpp>

int main(int argc, char** argv)
{
	std::cout.setf(std::ios_base::boolalpha);

	std::string ip = "10.0.0.1:12345";
	if(argc > 1) ip = std::string(argv[1]);

	neatoc::Controller controller;
	std::cout << "Hello NeatoC!" << std::endl;

	std::cout << "Connecting to " << ip << std::endl;
	controller.connect(ip);

	controller.setTestMode(true);
	std::cout << "Test mode: " << controller.getTestMode() << std::endl;

	controller.setLdsRotation(true);
	std::cout << "LDS rotation: " << controller.getLdsRotation() << std::endl;

	std::this_thread::sleep_for(std::chrono::seconds(5));

	neatoc::ScanData data = controller.getLdsScan();
	std::cout << "LDS data:" << std::endl << data << std::endl;

	controller.setLdsRotation(false);
	std::cout << "LDS rotation: " << controller.getLdsRotation() << std::endl;

	controller.setTestMode(false);
	std::cout << "Test mode: " << controller.getTestMode() << std::endl;

	return 0;
}
