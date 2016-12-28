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

#include "regilo/utils.hpp"

namespace regilo {

std::istream& getLine(std::istream& stream, std::string& line, const std::string& delim)
{
    if(delim.empty()) return std::getline(stream, line);
    if(delim.size() == 1) return std::getline(stream, line, delim.front());

    char character;
    stream.get(character);

    std::string delimPart, result;
    while(stream)
    {
        if(character == delim.at(delimPart.size()))
        {
            delimPart += character;
            if(delimPart.size() == delim.size())
            {
                delimPart.clear();
                break;
            }
        }
        else
        {
            if(!delimPart.empty())
            {
                result += delimPart;
                delimPart.clear();
            }

            result += character;
        }

        if(stream.peek() == EOF) break;
        else stream.get(character);
    }

    if(!delimPart.empty()) result += delimPart;
    if(!result.empty()) line = result;

    return stream;
}

}
