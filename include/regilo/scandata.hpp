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

#ifndef REGILO_SCANDATA_HPP
#define REGILO_SCANDATA_HPP

#include <vector>

#include "scanrecord.hpp"

namespace regilo {

/**
 * @brief The ScanData class is used to store laser data.
 */
class ScanData : public std::vector<ScanRecord>
{
public:
	std::size_t scanId; ///< The scan id (starting from zero).
	double rotationSpeed; ///< The rotation speed (in Hz).
	long time; ///< The scan time (seconds since epoch).

	/**
	 * @brief Default constructor.
	 */
	ScanData();

	/**
	 * @brief Construct ScanData.
	 * @param scanId The scan id (starting from zero).
	 * @param rotationSpeed The rotation speed (in Hz).
	 */
	ScanData(std::size_t scanId, double rotationSpeed);

	/**
	 * @brief Output the data as a string.
	 */
	friend std::ostream& operator<<(std::ostream& out, const ScanData& record);
};

}

#endif // REGILO_SCANDATA_HPP
