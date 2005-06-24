/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/mutex.h"
#include "common/system.h"

namespace Common {

Mutex::Mutex() {
	_mutex = g_system->createMutex();
}

Mutex::~Mutex() {
	g_system->deleteMutex(_mutex);
}

void Mutex::lock() {
	g_system->lockMutex(_mutex);
}

void Mutex::unlock() {
	g_system->unlockMutex(_mutex);
}


#pragma mark -


StackLock::StackLock(MutexRef mutex, const char *mutexName)
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

void StackLock::lock() {
	if (_mutexName != NULL)
		debug(6, "Locking mutex %s", _mutexName);
	
	g_system->lockMutex(_mutex);
}

void StackLock::unlock() {
	if (_mutexName != NULL)
		debug(6, "Unlocking mutex %s", _mutexName);

	g_system->unlockMutex(_mutex);
}

}	// End of namespace Common
