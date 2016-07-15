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

#ifndef SOCKETSIMULATOR_HPP
#define SOCKETSIMULATOR_HPP

#include <iostream>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include "regilo/log.hpp"

#include "simulator.hpp"

namespace bai = boost::asio::ip;

class SocketSimulator : public Simulator
{
private:
	boost::asio::io_service ioService;
	bai::tcp::acceptor acceptor;
	bai::tcp::socket socket;

	boost::asio::streambuf socketIStreamBuffer, socketOStreamBuffer;
	std::istream socketIStream;
	std::ostream socketOStream;

	virtual std::string read() override;
	virtual bool write(const std::string& data) override;

public:
	SocketSimulator(std::iostream& stream, unsigned short port);
	virtual ~SocketSimulator();

	virtual void start() override;
	virtual void stop() override;

	virtual bool isRunning() const override;
	virtual std::string getEndpoint() const override;

	virtual bool run() override;
};

#endif // SOCKETSIMULATOR_HPP
