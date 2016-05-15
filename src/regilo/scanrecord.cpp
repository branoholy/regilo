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

#include "regilo/scanrecord.hpp"

#include <cmath>
#include <iostream>

namespace regilo {

ScanRecord::ScanRecord(int id, double angle, double distance, int intensity, int errorCode, bool error) :
	id(id),
	angle(angle),
	distance(distance),
	intensity(intensity),
	errorCode(errorCode),
	error(error)
{
}

std::ostream& operator<<(std::ostream& out, const ScanRecord& record)
{
	out << "ScanRecord("
		<< record.id
		<< ": "
		<< record.angle * 180 * M_1_PI
		<< "°; "
		<< record.distance
		<< "mm";

	if(record.error) out << "; error";

	out << ')';

	return out;
}

}
