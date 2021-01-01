/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_TIMER_H
#define COMMON_TIMER_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/noncopyable.h"

namespace Common {

/**
 * @defgroup common_timer Timer
 * @ingroup common
 *
 * @brief API for managing the timer.
 *
 * @{
 */

class TimerManager : NonCopyable {
public:
	typedef void (*TimerProc)(void *refCon); /*!< Type definition of a timer instance. */

	virtual ~TimerManager() {}

	/**
	 * Install a new timer callback.
	 *
	 * After it has been created, the timer is called every @p interval microseconds.
	 * The timer can be invoked from a separate thread. Hence any timer code should be
	 * written following the same safety guidelines as any other threaded code.
	 *
	 * @note Although the interval is specified in microseconds, the actual timer resolution
	 *       may be lower. In particular, with the SDL backend the timer resolution is 10 ms.
	 *
	 * @param proc		Callback.
	 * @param interval	Interval in which the timer shall be invoked (in microseconds).
	 * @param refCon	Arbitrary void pointer passed to the timer callback.
	 * @param id        Unique string ID of the installed timer. Used by the event recorder.
	 *
	 * @return	True if the timer was installed successfully, false otherwise.
	 */
	virtual bool installTimerProc(TimerProc proc, int32 interval, void *refCon, const Common::String &id) = 0;

	/**
	 * Remove the given timer callback.
	 *
	 * It will not be invoked anymore, and no instance
	 * of this callback will be running anymore.
	 */
	virtual void removeTimerProc(TimerProc proc) = 0;
};

/** @} */

} // End of namespace Common

#endif
