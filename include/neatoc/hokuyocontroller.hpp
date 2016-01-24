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

#ifndef NEATOC_HOKUYOCONTROLLER_HPP
#define NEATOC_HOKUYOCONTROLLER_HPP

#include <map>

#include <boost/asio/serial_port.hpp>

#include "controller.hpp"

namespace neatoc {

/**
 * @brief The HokuyoController class is used to communicate with the Hokuyo scanner.
 */
class HokuyoController : public BaseController<ba::serial_port>
{
private:
	std::string endpoint;

	std::size_t validFromStep, validToStep;
	std::size_t maxStep;
	std::size_t fromStep, toStep, clusterCount;
	double startAngle;

protected:
	void init();

	inline std::string getScanCommand() const override { return createCommand(CMD_GET_SCAN, fromStep, toStep, clusterCount); }
	bool parseScanData(std::istream& in, ScanData& data);

public:
	static std::string CMD_GET_VERSION;
	static std::string CMD_GET_SCAN;

	/**
	 * @brief Default constructor.
	 */
	HokuyoController();

	/**
	 * @brief Controller with a log file specified by a path.
	 * @param logPath Path to the log file.
	 */
	HokuyoController(const std::string& logPath);

	/**
	 * @brief Controller with a log specified by a stream.
	 * @param logStream The log stream.
	 */
	HokuyoController(std::iostream& logStream);

	/**
	 * @brief Connect the controller to the Hokuyo scanner.
	 * @param endpoint The endpoint with the path to the device (e.g. "/dev/ttyACM0").
	 */
	void connect(const std::string& endpoint);

	/**
	 * @brief Get the endpoint of the Hokuyo scanner.
	 * @return The endpoint.
	 */
	inline std::string getEndpoint() const { return endpoint; }

	/**
	 * @brief Return information about version.
	 * @return Key-value pairs with the information
	 */
	std::map<std::string, std::string> getVersionInfo();

	/**
	 * @brief Set parameters for the scan command.
	 * @param fromStep The starting step [0; maxStep].
	 * @param toStep The ending steop [0; maxStep].
	 * @param clusterCount The cluster count [0; 99].
	 */
	void setScanParameters(std::size_t fromStep, std::size_t toStep, std::size_t clusterCount);
};

}

#endif // NEATOC_HOKUYOCONTROLLER_HPP
