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

#ifndef REGILO_SERIALCONTROLLER_HPP
#define REGILO_SERIALCONTROLLER_HPP

#include <boost/asio/serial_port.hpp>

#include "streamcontroller.hpp"

namespace regilo {

/**
 * @brief The SerialController class is used to communicate with a device using the serial port.
 */
class SerialController : public StreamController<ba::serial_port>
{
private:
	std::string endpoint;

public:
	using StreamController::StreamController;

	/**
	 * @brief Connect the controller to a device.
	 * @param endpoint The endpoint with the path to the device (e.g. "/dev/ttyACM0").
	 */
	virtual void connect(const std::string& endpoint) override;

	virtual inline std::string getEndpoint() const override { return endpoint; }
};

}

#endif // REGILO_SERIALCONTROLLER_HPP
