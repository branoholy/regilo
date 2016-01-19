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

#ifndef NEATOC_SCANRECORD_HPP
#define NEATOC_SCANRECORD_HPP

#include <iostream>

namespace neatoc {

/**
 * @brief The ScanRecord class represents one record from LDS data.
 */
class ScanRecord
{
public:
	int id; ///< The id of the record (starting from zero).
	double angle; ///< The angle of the record (in radians).
	double distance; ///< The distance that was measured in the angle (in millimeters).
	int intensity; ///< The normalized spot intensity that was measured in the angle.
	int errorCode; ///< The error code (zero means no error).

	/**
	 * @brief Default constructor.
	 */
	ScanRecord();

	/**
	 * @brief Construct a ScanRecord from all attributes.
	 * @param id The id of the record (starting from zero).
	 * @param angle The angle of the record (in radians).
	 * @param distance The distance that was measured in the angle (in millimeters).
	 * @param intensity The normalized spot intensity that was measured in the angle.
	 * @param errorCode The error code (zero means no error).
	 */
	ScanRecord(int id, double angle, double distance, int intensity, int errorCode);

	/**
	 * @brief Output the record as a string.
	 */
	friend std::ostream& operator<<(std::ostream& stream, const ScanRecord& record);
};

}

#endif // NEATOC_SCANRECORD_HPP
