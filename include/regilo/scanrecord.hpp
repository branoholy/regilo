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

#ifndef REGILO_SCANRECORD_HPP
#define REGILO_SCANRECORD_HPP

#include <iosfwd>

namespace regilo {

/**
 * @brief The ScanRecord class represents one record from laser data.
 */
class ScanRecord
{
public:
	int id; ///< The id of the record (starting from zero).
	double angle; ///< The angle of the record (in radians).
	double distance; ///< The distance that was measured in the angle (in millimeters).
	int intensity; ///< The normalized spot intensity that was measured in the angle.
	int errorCode; ///< The error code.
	bool error; ///< True if this record has an error.

	/**
	 * @brief Default constructor.
	 */
	ScanRecord() = default;

	/**
	 * @brief Construct a ScanRecord from all attributes.
	 * @param id The id of the record (starting from zero).
	 * @param angle The angle of the record (in radians).
	 * @param distance The distance that was measured in the angle (in millimeters).
	 * @param intensity The normalized spot intensity that was measured in the angle.
	 * @param errorCode The error code.
	 * @param error True if this record has an error.
	 */
	ScanRecord(int id, double angle, double distance, int intensity, int errorCode, bool error = false);

	/**
	 * @brief Output the record as a string.
	 */
	friend std::ostream& operator<<(std::ostream& out, const ScanRecord& record);
};

}

#endif // REGILO_SCANRECORD_HPP
