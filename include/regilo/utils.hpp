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

#ifndef REGILO_UTILS_HPP
#define REGILO_UTILS_HPP

#include <chrono>

namespace regilo {

/**
 * @brief Get time since epoch.
 * @return Time as std::duration
 */
template<typename T>
T epoch()
{
	auto sinceEpoch = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<T>(sinceEpoch);
}

}

#endif // REGILO_UTILS_HPP
