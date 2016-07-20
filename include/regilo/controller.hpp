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

#ifndef REGILO_CONTROLLER_HPP
#define REGILO_CONTROLLER_HPP

#include <memory>

#include "log.hpp"

namespace regilo {

/**
 * @brief The IController interface is used for all controller classes.
 */
class IController
{
public:
	/**
	 * @brief Default destructor.
	 */
	virtual ~IController() = default;

	/**
	 * @brief Connect the controller to a device.
	 * @param endpoint The device endpoint (a path, IP address, etc.).
	 */
	virtual void connect(const std::string& endpoint) = 0;

	/**
	 * @brief Test if the controller is connected.
	 * @return True if connected.
	 */
	virtual bool isConnected() const = 0;

	/**
	 * @brief Get the endpoint of device.
	 * @return The device endpoint or empty string.
	 */
	virtual std::string getEndpoint() const = 0;

	/**
	 * @brief Get the current Log.
	 * @return The Log or empty std::shared_ptr.
	 */
	virtual std::shared_ptr<ILog> getLog() = 0;

	/**
	 * @brief Get the current Log (a const variant).
	 * @return The Log or empty std::shared_ptr.
	 */
	virtual std::shared_ptr<const ILog> getLog() const = 0;

	/**
	 * @brief Set a Log (it can be shared between more controllers).
	 * @param log Smart pointer to the Log.
	 */
	virtual void setLog(std::shared_ptr<ILog> log) = 0;

	/**
	 * @brief Send a command to the device.
	 * @param command A command with all parameters.
	 * @return A string with a whole response to the command.
	 */
	virtual std::string sendCommand(const std::string& command) = 0;
};

}

#endif // REGILO_CONTROLLER_HPP
