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

#ifndef REGILO_SCANCONTROLLER_HPP
#define REGILO_SCANCONTROLLER_HPP

#include "controller.hpp"
#include "scandata.hpp"
#include "utils.hpp"

namespace regilo {

/**
 * @brief The IScanController interface is used for all controller classes that implement scanning functionality.
 */
class IScanController : public virtual IController
{
public:
	/**
	 * @brief Default destructor.
	 */
	virtual ~IScanController() = default;

	/**
	 * @brief Get a scan from the device.
	 * @param fromDevice Specify if you want to get a scan from the device (true) or log (false). Default: true.
	 * @return ScanData
	 */
	virtual ScanData getScan(bool fromDevice = true) = 0;
};

/**
 * @brief The ScanController class implements parsing of scanned laser data.
 */
template<typename ProtocolController>
class ScanController : public IScanController, public ProtocolController
{
protected:
	std::size_t lastScanId = 0; ///< A scan id (starting from zero) that is used for new scans.

	/**
	 * @brief Get a string that can be used for getting a scan.
	 * @return A command for getting a scan.
	 */
	virtual std::string getScanCommand() const = 0;

	/**
	 * @brief Parse the raw scan data.
	 * @param in The input stream that stores the raw scan data.
	 * @param data Output for the scanned data.
	 * @return True if the parsing ends without an error.
	 */
	virtual bool parseScanData(std::istream& in, ScanData& data) = 0;

public:
	using ProtocolController::ProtocolController;

	/**
	 * @brief Default destructor.
	 */
	virtual ~ScanController() = default;

	virtual ScanData getScan(bool fromDevice = true) override final;
};

template<typename ProtocolController>
ScanData ScanController<ProtocolController>::getScan(bool fromDevice)
{
	ScanData data;

	if(fromDevice)
	{
		this->sendCommand(getScanCommand());
		data.time = epoch<std::chrono::milliseconds>().count();

		parseScanData(this->deviceOutput, data);
	}
	else
	{
		std::istringstream response(this->log->readCommand(getScanCommand()));
		if(std::shared_ptr<const ITimedLog> timedLog = std::dynamic_pointer_cast<const ITimedLog>(this->getLog()))
		{
			data.time = timedLog->getLastCommandTimeAs<std::chrono::milliseconds>().count();
		}

		parseScanData(response, data);
	}
	if(!data.empty()) data.scanId = lastScanId++;

	return data;
}

}

#endif // REGILO_SCANCONTROLLER_HPP
