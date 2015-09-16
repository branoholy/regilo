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

#include "neatoc/scanrecord.hpp"

#include <cmath>

neatoc::ScanRecord::ScanRecord()
{
}

neatoc::ScanRecord::ScanRecord(int id, double angle, double distance, int intensity, int errorCode) :
	id(id),
	angle(angle),
	distance(distance),
	intensity(intensity),
	errorCode(errorCode)
{
}

namespace neatoc {

std::ostream& operator<<(std::ostream& stream, const neatoc::ScanRecord& record)
{
	return stream << "ScanRecord("
		   << record.id
		   << ": "
		   << record.angle * 180 * M_1_PI
		   << "°; "
		   << record.distance
		   << "mm"
		   << ')';
}

}
