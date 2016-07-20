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

template<typename DurationT>
class TimedLog;

/**
 * @brief The ITimedLogMetadata interface is used for TimedLogMetadata.
 */
class ITimedLogMetadata
{
public:
	/**
	 * @brief Get the num of ratio.
	 * @return The num value.
	 */
	virtual std::intmax_t getNum() const = 0;

	/**
	 * @brief Get the den of ratio.
	 * @return The den value.
	 */
	virtual std::intmax_t getDen() const = 0;
};

/**
 * @brief The TimedLogMetadata class specifies the metadata for TimedLog.
 */
template<typename DurationT>
class TimedLogMetadata : public LogMetadata, public ITimedLogMetadata
{
private:
	std::intmax_t num = DurationT::period::num;
	std::intmax_t den = DurationT::period::den;

protected:
	/**
	 * @brief The default constructor is available only in derived and friend classes.
	 */
	TimedLogMetadata();

public:
	/**
	 * @brief The default destructor.
	 */
	virtual ~TimedLogMetadata() = default;

	inline virtual std::intmax_t getNum() const override final { return num; }
	inline virtual std::intmax_t getDen() const override final { return den; }

	friend class TimedLog<DurationT>;
};

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
	 * @brief Get the associated timed metadata.
	 * @return A pointer to timed metadata
	 */
	virtual const ITimedLogMetadata* getTimedMetadata() const = 0;

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

	DurationT lastCommandTime;

	DurationT firstReadTime = DurationT::zero();
	DurationT firstWriteTime = DurationT::min();

protected:
	virtual void readMetadata(std::istream& metaStream) override;
	virtual void writeMetadata(std::ostream& metaStream) override;

	virtual std::string readData(std::string& logCommand) override;
	virtual void writeData(const std::string& command, const std::string& response) override;

public:
	typedef DurationT Duration; ///< The duration type for this log.

	using Log::Log;

	/**
	 * @brief The default destructor.
	 */
	virtual ~TimedLog() = default;

	virtual inline const ITimedLogMetadata* getTimedMetadata() const override { return (TimedLogMetadata<DurationT>*)metadata; }

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
};

extern template class TimedLog<std::chrono::nanoseconds>;
extern template class TimedLog<std::chrono::microseconds>;
extern template class TimedLog<std::chrono::milliseconds>;
extern template class TimedLog<std::chrono::seconds>;

template<typename DurationT>
TimedLogMetadata<DurationT>::TimedLogMetadata() :
	LogMetadata("timedlog", 2)
{
}

template<typename DurationT>
void TimedLog<DurationT>::readMetadata(std::istream& metaStream)
{
	if(metadata == nullptr) metadata = new TimedLogMetadata<DurationT>();
	Log::readMetadata(metaStream);

	readName(metaStream, "timeres");
	TimedLogMetadata<DurationT> *timedMetadata = (TimedLogMetadata<DurationT>*)metadata;
	metaStream >> timedMetadata->num >> timedMetadata->den;
}

template<typename DurationT>
void TimedLog<DurationT>::writeMetadata(std::ostream& metaStream)
{
	if(metadata == nullptr) metadata = new TimedLogMetadata<DurationT>();
	Log::writeMetadata(metaStream);

	metaStream << "timeres " << DurationT::period::num << ' ' << DurationT::period::den << std::endl;
}

template<typename DurationT>
std::string TimedLog<DurationT>::readData(std::string& logCommand)
{
	std::lock_guard<std::mutex> streamLock(streamMutex);

	std::string response = Log::readData(logCommand);

	std::int64_t commandTimeCount = 0;
	readName(stream, 't');
	stream >> commandTimeCount;

	if(stream)
	{
		long double numRatio = getTimedMetadata()->getNum() / (long double) DurationT::period::num;
		long double denRation = DurationT::period::den / (long double) getTimedMetadata()->getDen();
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
	}

	return response;
}

template<typename DurationT>
void TimedLog<DurationT>::writeData(const std::string& command, const std::string& response)
{
	std::lock_guard<std::mutex> streamLock(streamMutex);

	Log::writeData(command, response);

	if(firstWriteTime == DurationT::min()) firstWriteTime = epoch<DurationT>();
	stream << "t " << (epoch<DurationT>() - firstWriteTime).count() << std::endl;
}

}

#endif // REGILO_TIMEDLOG_HPP
