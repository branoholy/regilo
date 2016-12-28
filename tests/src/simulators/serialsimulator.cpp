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

#include "simulators/serialsimulator.hpp"

#include <unistd.h>

SerialSimulator::~SerialSimulator()
{
    stop();
}

void SerialSimulator::start()
{
    if(!opened)
    {
        ptmx = ::open("/dev/ptmx", O_RDWR | O_NOCTTY);
        opened = (ptmx != -1 && grantpt(ptmx) == 0 && unlockpt(ptmx) == 0);
    }
}

void SerialSimulator::stop()
{
    if(opened)
    {
        ::close(ptmx);
        opened = false;
    }
}

std::string SerialSimulator::read(std::size_t bufferSize)
{
    std::string response;
    char *buffer = new char[bufferSize];

    ssize_t readBytes;
    while((readBytes = ::read(ptmx, buffer, bufferSize - 1)) > 0)
    {
        if(!requestEnd.empty())
        {
            std::string bufferString(buffer, readBytes);
            std::size_t requestEndPos = bufferString.find(requestEnd);

            if(requestEndPos != std::string::npos)
            {
                response.append(buffer, requestEndPos + 1);
                delete[] buffer;

                return response;
            }
        }

        response.append(buffer, readBytes);
    }

    return response;
}

bool SerialSimulator::write(const std::string& data)
{
    ssize_t writtenBytes = ::write(ptmx, data.c_str(), data.length());
    return writtenBytes != -1 && std::size_t(writtenBytes) == data.length();
}
