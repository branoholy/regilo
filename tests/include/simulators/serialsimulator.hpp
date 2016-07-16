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

#ifndef SERIALSIMULATOR_HPP
#define SERIALSIMULATOR_HPP

#include <iostream>
#include <fcntl.h>

#include "regilo/log.hpp"

#include "simulator.hpp"

class SerialSimulator : public Simulator
{
private:
	int ptmx;
	bool opened = false;

protected:
	inline virtual std::string read() override { return read(256); }
	std::string read(std::size_t bufferSize);
	virtual bool write(const std::string& data) override;

public:
	using Simulator::Simulator;
	virtual ~SerialSimulator();

	virtual void start() override;
	virtual void stop() override;

	inline virtual bool isRunning() const override { return opened; }
	inline virtual std::string getEndpoint() const override { return std::string(ptsname(ptmx)); }
};

#endif // SERIALSIMULATOR_HPP
