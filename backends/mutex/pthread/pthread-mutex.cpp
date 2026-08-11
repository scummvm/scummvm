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

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "backends/mutex/pthread/pthread-mutex.h"

#include <pthread.h>

/**
 * pthreads mutex implementation
 */
class PthreadMutexInternal final : public Common::MutexInternal {
public:
	PthreadMutexInternal();
	~PthreadMutexInternal() override;

	bool lock() override;
	bool unlock() override;

private:
	pthread_mutex_t _mutex;
};


PthreadMutexInternal::PthreadMutexInternal() {
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	if (pthread_mutex_init(&_mutex, &attr) != 0) {
		warning("pthread_mutex_init() failed");
	}
}

PthreadMutexInternal::~PthreadMutexInternal() {
	if (pthread_mutex_destroy(&_mutex) != 0)
		warning("pthread_mutex_destroy() failed");
}

bool PthreadMutexInternal::lock() {
	if (pthread_mutex_lock(&_mutex) != 0) {
		warning("pthread_mutex_lock() failed");
		return false;
	} else {
		return true;
	}
}

bool PthreadMutexInternal::unlock() {
	if (pthread_mutex_unlock(&_mutex) != 0) {
		warning("pthread_mutex_unlock() failed");
		return false;
	} else {
		return true;
	}
}

Common::MutexInternal *createPthreadMutexInternal() {
	return new PthreadMutexInternal();
}
