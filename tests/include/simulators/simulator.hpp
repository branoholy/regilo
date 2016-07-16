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

#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <iostream>

#include "regilo/log.hpp"

class Simulator
{
protected:
	regilo::Log log;

	virtual std::string read() = 0;
	virtual bool write(const std::string& data) = 0;

public:
	std::string requestEnd = "\n";
	std::string responseEnd = "\n";

	Simulator(const std::string& filePath);
	Simulator(std::iostream& stream);

	virtual ~Simulator() = default;

	virtual void start() = 0;
	virtual void stop() = 0;

	virtual bool isRunning() const = 0;
	virtual std::string getEndpoint() const = 0;

	virtual bool run();
};

#endif // SIMULATOR_HPP
