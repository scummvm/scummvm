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

#ifndef SINGLE_THREADED

#include "common/debug.h"
#include "common/mutex.h"
#include "common/system.h"

namespace Common {

Mutex::Mutex() {
	assert(g_system);
	_mutex = g_system->createMutex();
}

Mutex::~Mutex() {
	delete _mutex;
}

bool Mutex::lock() {
	return _mutex->lock();
}

bool Mutex::unlock() {
	return _mutex->unlock();
}


#pragma mark -


StackLock::StackLock(MutexInternal *mutex, const char *mutexName)
	: _mutex(mutex), _mutexName(mutexName) {
	lock();
}

StackLock::StackLock(const Mutex &mutex, const char *mutexName)
	: _mutex(mutex._mutex), _mutexName(mutexName) {
	lock();
}

StackLock::~StackLock() {
	unlock();
}

bool StackLock::lock() {
	if (_mutexName != nullptr)
		debug(6, "Locking mutex %s", _mutexName);

	return _mutex->lock();
}

bool StackLock::unlock() {
	if (_mutexName != nullptr)
		debug(6, "Unlocking mutex %s", _mutexName);

	return _mutex->unlock();
}

} // End of namespace Common

#endif
