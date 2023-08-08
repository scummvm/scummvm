/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_MUTEX_H
#define COMMON_MUTEX_H

#include "common/scummsys.h"
#include "common/system.h"

namespace Common {

/**
 * @defgroup common_mutex Mutex
 * @ingroup common
 *
 * @brief API for managing the mutex.
 * @{
 */

class Mutex;

class MutexInternal {
public:
	virtual ~MutexInternal() {}

	virtual bool lock() = 0;
	virtual bool unlock() = 0;
};

#ifndef SINGLE_THREADED
/**
 * Auxillary class to (un)lock a mutex on the stack.
 */
class StackLock {
	MutexInternal *_mutex;
	const char *_mutexName;

	bool lock();
	bool unlock();
public:
	explicit StackLock(MutexInternal *mutex, const char *mutexName = nullptr);
	explicit StackLock(const Mutex &mutex, const char *mutexName = nullptr);
	~StackLock();
};


/**
 * Wrapper class around the OSystem mutex functions.
 */
class Mutex {
	friend class StackLock;

	MutexInternal *_mutex;

public:
	Mutex();
	~Mutex();

	bool lock();
	bool unlock();
};

/** @} */

#else

class StackLock final {
public:
	explicit StackLock(MutexInternal *mutex, const char *mutexName = nullptr) {}
	explicit StackLock(const Mutex &mutex, const char *mutexName = nullptr) {}
};

class Mutex final {
public:
	void lock() {}
	void unlock() {}
};

#endif

} // End of namespace Common

#endif
