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

#include "regilo/socketcontroller.hpp"

namespace regilo {

SocketController::~SocketController()
{
	if(stream.is_open())
	{
		stream.shutdown(bai::tcp::socket::shutdown_both);
	}
}

void SocketController::connect(const std::string& endpoint)
{
	std::string ip = endpoint;
	unsigned short port = 0;

	std::size_t colonPos = endpoint.find(':');
	if(colonPos != std::string::npos)
	{
		ip = endpoint.substr(0, colonPos);
		port = static_cast<unsigned short>(std::stoul(endpoint.substr(colonPos + 1)));
	}

	connect(ip, port);
}

void SocketController::connect(const std::string& ip, unsigned short port)
{
	connect(bai::tcp::endpoint(bai::address::from_string(ip), port));
}

void SocketController::connect(const bai::tcp::endpoint& endpoint)
{
	stream.connect(endpoint);
}

std::string SocketController::getEndpoint() const
{
	if(!isConnected()) return "";

	Stream::endpoint_type endpoint = stream.remote_endpoint();

	std::string ip = endpoint.address().to_string();
	std::string port = std::to_string(endpoint.port());

	return ip + ':' + port;
}

}
