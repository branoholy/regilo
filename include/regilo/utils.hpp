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
#include <iostream>

namespace regilo {

/**
 * @brief Get time since epoch.
 * @return Time as std::duration.
 */
template<typename T>
T epoch()
{
    auto sinceEpoch = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<T>(sinceEpoch);
}

/**
 * @brief Get a line from a stream with a multi-char delimiter.
 * @param stream A stream from which characters are extracted.
 * @param line A string where the extracted line is stored.
 * @param delim A string that is used as a delimiter.
 * @return The input stream.
 */
std::istream& getLine(std::istream& stream, std::string& line, const std::string& delim);

}

#endif // REGILO_UTILS_HPP
