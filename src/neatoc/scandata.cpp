/*
 * NeatoC
 * Copyright (C) 2015  Branislav Holý <branoholy@gmail.com>
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

#include "neatoc/scandata.hpp"

#include <cmath>

#include <boost/algorithm/string.hpp>

#include "neatoc/controller.hpp"

neatoc::ScanData::ScanData() :
	scanId(-1), rotationSpeed(-1)
{
}

neatoc::ScanData::ScanData(std::size_t scanId, double rotationSpeed) :
	scanId(scanId), rotationSpeed(rotationSpeed)
{
}

namespace neatoc {

std::istream& operator>>(std::istream& stream, neatoc::ScanData& data)
{
	int lastId = 0;
	double M_PI_180 = M_PI / 180.0;

	std::string line;
	std::getline(stream, line);
	boost::algorithm::trim(line);

	if(line == Controller::LDS_SCAN_HEADER)
	{
		while(true)
		{
			std::getline(stream, line);
			boost::algorithm::trim(line);

			if(boost::algorithm::starts_with(line, Controller::LDS_SCAN_FOOTER))
			{
				std::vector<std::string> values;
				boost::algorithm::split(values, line, boost::algorithm::is_any_of(","));
				data.rotationSpeed = std::stod(values.at(1));

				break;
			}
			else
			{
				std::vector<std::string> values;
				boost::algorithm::split(values, line, boost::algorithm::is_any_of(","));

				int id = lastId++;
				double angle = std::stod(values.at(0)) * M_PI_180;
				double distance = std::stod(values.at(1));
				int intensity = std::stoi(values.at(2));
				int errorCode = std::stoi(values.at(3));

				data.emplace_back(id, angle, distance, intensity, errorCode);
			}
		}
	}

	return stream;
}

std::ostream& operator<<(std::ostream& stream, const neatoc::ScanData& data)
{
	stream << "ScanData("
		   << data.scanId
		   << ", "
		   << data.rotationSpeed
		   << ", "
		   << data.size()
		   << ')'
		   << std::endl;

	for(const neatoc::ScanRecord& record : data)
	{
		stream << record << std::endl;
	}

	return stream;
}

}
