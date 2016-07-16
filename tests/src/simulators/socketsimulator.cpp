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

#include "simulators/socketsimulator.hpp"

#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>

#include "regilo/utils.hpp"

SocketSimulator::SocketSimulator(const std::string& filePath, unsigned short port) : Simulator(filePath),
	acceptor(ioService, bai::tcp::endpoint(bai::tcp::v4(), port)),
	socket(ioService),
	socketIStream(&socketIStreamBuffer),
	socketOStream(&socketOStreamBuffer)
{
}

SocketSimulator::SocketSimulator(std::iostream& stream, unsigned short port) : Simulator(stream),
	acceptor(ioService, bai::tcp::endpoint(bai::tcp::v4(), port)),
	socket(ioService),
	socketIStream(&socketIStreamBuffer),
	socketOStream(&socketOStreamBuffer)
{
}

SocketSimulator::~SocketSimulator()
{
	stop();
}

void SocketSimulator::start()
{
}

void SocketSimulator::stop()
{
	if(isRunning()) acceptor.close();
}

bool SocketSimulator::isRunning() const
{
	return (acceptor.is_open() && socket.is_open());
}

std::string SocketSimulator::getEndpoint() const
{
	return "127.0.0.1:" + std::to_string(acceptor.local_endpoint().port());
}

bool SocketSimulator::run()
{
	acceptor.accept(socket);
	return Simulator::run();
}

std::string SocketSimulator::read()
{
	std::string response;

	boost::system::error_code errorCode;
	boost::asio::read_until(socket, socketIStreamBuffer, requestEnd, errorCode);

	if(errorCode == boost::asio::error::eof) return "";

	regilo::getLine(socketIStream, response, requestEnd);

	return response + requestEnd;
}

bool SocketSimulator::write(const std::string& data)
{
	socketOStream << data;

	try
	{
		size_t writtenBytes = boost::asio::write(socket, socketOStreamBuffer);
		return (writtenBytes == data.length());
	}
	catch(boost::system::system_error&)
	{
		return false;
	}
}
