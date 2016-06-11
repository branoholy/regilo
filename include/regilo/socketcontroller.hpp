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

#ifndef REGILO_SOCKETCONTROLLER_HPP
#define REGILO_SOCKETCONTROLLER_HPP

#include <boost/asio/ip/tcp.hpp>

#include "controller.hpp"

namespace regilo {

namespace bai = ba::ip;

/**
 * @brief The SocketController class is used to communicate with a device using a TCP socket.
 */
class SocketController : public StreamController<bai::tcp::socket>
{
public:
	using StreamController::StreamController;

	/**
	 * @brief Default destructor.
	 */
	virtual ~SocketController();

	/**
	 * @brief Connect the controller to a device.
	 * @param endpoint The endpoint with the IP address and port of the device (e.g. "10.0.0.1:12345").
	 */
	virtual void connect(const std::string& endpoint) override;

	/**
	 * @brief Connect the controller to a device.
	 * @param ip The IP address of the device (e.g. "10.0.0.1").
	 * @param port The port number of the device (e.g. 12345).
	 */
	void connect(const std::string& ip, unsigned short port);

	/**
	 * @brief Connect the controller to a device.
	 * @param endpoint The endpoint of the device.
	 */
	void connect(const bai::tcp::endpoint& endpoint);

	virtual std::string getEndpoint() const override;
};

}

#endif // REGILO_SOCKETCONTROLLER_HPP
