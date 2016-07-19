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

#ifndef REGILO_TIMEDLOG_HPP
#define REGILO_TIMEDLOG_HPP

#include <chrono>
#include <cmath>
#include <sstream>
#include <thread>

#include "regilo/log.hpp"
#include "regilo/utils.hpp"

namespace regilo {

/**
 * @brief The ITimedLog interface is implemented in TimedLog.
 */
class ITimedLog : public virtual ILog
{
public:
	/**
	 * @brief Default destructor.
	 */
	virtual ~ITimedLog() = default;

	/**
	 * @brief Get the last command time (after reading).
	 * @return Time since epoch as std::chrono::nanoseconds.
	 */
	virtual std::chrono::nanoseconds getLastCommandNanoseconds() const = 0;

	/**
	 * @brief Get the last command time (after reading).
	 * @return Time since epoch as Duration.
	 */
	template<typename Duration>
	inline Duration getLastCommandTimeAs() const { return std::chrono::duration_cast<Duration>(this->getLastCommandNanoseconds()); }

	/**
	 * @brief Sync command times with real time. It means that all read methods will block
	 *        their executions until the current time is bigger than the command time.
	 */
	virtual void syncTime(bool sync = true) = 0;
};

/**
 * @brief The TimedLog class is used to log all commands with their timestamp.
 */
template<typename DurationT = std::chrono::milliseconds>
class TimedLog : public Log, public ITimedLog
{
private:
	std::mutex streamMutex;

	std::intmax_t num, den;

	DurationT lastCommandTime;

	DurationT firstReadTime = DurationT::zero();
	DurationT firstWriteTime = DurationT::min();

protected:
	virtual void readMetadata(std::istream& metaStream) override;
	virtual void writeMetadata(std::ostream& metaStream) override;

public:
	typedef DurationT Duration; ///< The duration type for this log.

	using Log::Log;

	/**
	 * @brief Default destructor.
	 */
	virtual ~TimedLog() = default;

	inline virtual std::chrono::nanoseconds getLastCommandNanoseconds() const override
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(lastCommandTime);
	}

	/**
	 * @brief Get the last command time (after reading).
	 * @return Time since epoch as Duration.
	 */
	inline DurationT getLastCommandTime() const { return lastCommandTime; }

	virtual inline void syncTime(bool sync = true) override { firstReadTime = (sync ? DurationT::max() : DurationT::zero()); }

	virtual std::string read(std::string& logCommand) override;
	virtual void write(const std::string& command, const std::string& response) override;
};

extern template class TimedLog<std::chrono::nanoseconds>;
extern template class TimedLog<std::chrono::microseconds>;
extern template class TimedLog<std::chrono::milliseconds>;
extern template class TimedLog<std::chrono::seconds>;

template<typename DurationT>
void TimedLog<DurationT>::readMetadata(std::istream& metaStream)
{
	Log::readMetadata(metaStream);
	metaStream >> num >> den;
}

template<typename DurationT>
void TimedLog<DurationT>::writeMetadata(std::ostream& metaStream)
{
	Log::writeMetadata(metaStream);
	metaStream << ' ' << DurationT::period::num << ' ' << DurationT::period::den;
}

template<typename DurationT>
std::string TimedLog<DurationT>::read(std::string& logCommand)
{
	streamMutex.lock();

	std::string response = Log::read(logCommand);

	std::string epochTime;
	std::getline(stream, epochTime, MESSAGE_END);
	std::istringstream epochStream(epochTime);

	std::int64_t commandTimeCount;
	epochStream >> commandTimeCount;

	long double numRatio = num / DurationT::period::num;
	long double denRation = DurationT::period::den / den;
	lastCommandTime = DurationT(std::int64_t(std::round(commandTimeCount * numRatio * denRation)));

	if(firstReadTime == DurationT::max()) firstReadTime = epoch<DurationT>();
	else
	{
		DurationT elapsed = epoch<DurationT>() - firstReadTime;

		while(elapsed < lastCommandTime)
		{
			std::this_thread::sleep_for(lastCommandTime - elapsed);
			elapsed = epoch<DurationT>() - firstReadTime;
		}
	}

	streamMutex.unlock();

	return response;
}

template<typename DurationT>
void TimedLog<DurationT>::write(const std::string& command, const std::string& response)
{
	streamMutex.lock();

	Log::write(command, response);

	if(firstWriteTime == DurationT::min()) firstWriteTime = epoch<DurationT>();
	stream << (epoch<DurationT>() - firstWriteTime).count() << MESSAGE_END;

	streamMutex.unlock();
}

}

#endif // REGILO_TIMEDLOG_HPP
