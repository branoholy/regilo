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

class ScanController : public Controller
{
public:
	virtual ~ScanController() = default;

	virtual void connect(const std::string& endpoint) override = 0;
	virtual bool isConnected() const override = 0;
	virtual std::string getEndpoint() const override = 0;
	virtual std::shared_ptr<Log> getLog() override = 0;
	virtual std::shared_ptr<const Log> getLog() const override = 0;
	virtual void setLog(std::shared_ptr<Log> log) override = 0;

	/**
	 * @brief Get a scan from the device.
	 * @param fromDevice Specify if you want to get a scan from the device (true) or log (false). Default: true
	 * @return ScanData
	 */
	virtual ScanData getScan(bool fromDevice = true) = 0;
};

template<typename ProtocolController>
class BaseScanController : public ScanController, public ProtocolController
{
protected:
	std::size_t lastScanId;

	virtual std::string getScanCommand() const = 0;
	virtual bool parseScanData(std::istream& in, ScanData& data) = 0;

public:
	BaseScanController();
	BaseScanController(const std::string& logPath);
	BaseScanController(std::iostream& logStream);
	~BaseScanController() = default;

	virtual inline void connect(const std::string& endpoint) override { ProtocolController::connect(endpoint); }
	virtual inline bool isConnected() const override { return ProtocolController::isConnected(); }
	virtual inline std::string getEndpoint() const override { return ProtocolController::getEndpoint(); }
	virtual inline std::shared_ptr<Log> getLog() override { return ProtocolController::getLog(); }
	virtual inline std::shared_ptr<const Log> getLog() const override { return ProtocolController::getLog(); }
	virtual inline void setLog(std::shared_ptr<Log> log) override { return ProtocolController::setLog(log); }

	virtual ScanData getScan(bool fromDevice = true) override final;
};

template<typename ProtocolController>
BaseScanController<ProtocolController>::BaseScanController() : ProtocolController(),
	lastScanId(0)
{
}

template<typename ProtocolController>
BaseScanController<ProtocolController>::BaseScanController(const std::string& logPath) : ProtocolController(logPath),
	lastScanId(0)
{
}

template<typename ProtocolController>
BaseScanController<ProtocolController>::BaseScanController(std::iostream& logStream) : ProtocolController(logStream),
	lastScanId(0)
{
}

template<typename ProtocolController>
ScanData BaseScanController<ProtocolController>::getScan(bool fromDevice)
{
	ScanData data;

	if(fromDevice)
	{
		this->sendCommand(getScanCommand());
		data.time = epoch<std::chrono::seconds>();

		parseScanData(this->deviceOutput, data);
	}
	else
	{
		std::istringstream response(this->log->readCommand(getScanCommand()));
		// data.time = this->log->getLastCommandTime(); // TODO: Set from log

		parseScanData(response, data);
	}
	if(!data.empty()) data.scanId = lastScanId++;

	return data;
}

}

#endif // REGILO_SCANCONTROLLER_HPP
