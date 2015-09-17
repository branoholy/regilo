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

neatoc::ScanData::ScanData() :
	scanId(-1), rotationSpeed(-1)
{
}

neatoc::ScanData::ScanData(std::size_t scanId, double rotationSpeed) :
	scanId(scanId), rotationSpeed(rotationSpeed)
{
}

namespace neatoc {

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
