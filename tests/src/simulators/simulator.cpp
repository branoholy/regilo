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

#include "simulators/simulator.hpp"

Simulator::Simulator(const std::string& filePath) :
    log(filePath)
{
}

Simulator::Simulator(std::iostream& stream) :
    log(stream)
{
}

bool Simulator::run()
{
    bool status = false;
    while(isRunning())
    {
        status = true;

        std::string logCommand;
        std::string logResponse = log.read(logCommand);
        if(log.isEnd()) break;

        std::string devCommand = read();
        if(logCommand != devCommand) return false;

        if(!write(devCommand + logResponse)) return false;
    }

    return status;
}
