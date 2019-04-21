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

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "common/scummsys.h"

#if defined(__ANDROID__) || defined(IPHONE)

#include "backends/mutex/pthread/pthread-mutex.h"

#include <pthread.h>


OSystem::MutexRef PthreadMutexManager::createMutex() {
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_t *mutex = new pthread_mutex_t;

	if (pthread_mutex_init(mutex, &attr) != 0) {
		warning("pthread_mutex_init() failed");
		delete mutex;
		return NULL;
	}

	return (OSystem::MutexRef)mutex;
}

void PthreadMutexManager::lockMutex(OSystem::MutexRef mutex) {
	if (pthread_mutex_lock((pthread_mutex_t *)mutex) != 0)
		warning("pthread_mutex_lock() failed");
}

void PthreadMutexManager::unlockMutex(OSystem::MutexRef mutex) {
	if (pthread_mutex_unlock((pthread_mutex_t *)mutex) != 0)
		warning("pthread_mutex_unlock() failed");
}

void PthreadMutexManager::deleteMutex(OSystem::MutexRef mutex) {
	pthread_mutex_t *m = (pthread_mutex_t *)mutex;

	if (pthread_mutex_destroy(m) != 0)
		warning("pthread_mutex_destroy() failed");
	else
		delete m;
}

#endif
