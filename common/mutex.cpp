/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/mutex.h"
#include "common/debug.h"

#include "backends/platform/driver.h"

namespace Common {

Mutex::Mutex() {
	_mutex = g_driver->createMutex();
}

Mutex::~Mutex() {
	g_driver->deleteMutex(_mutex);
}

void Mutex::lock() {
	g_driver->lockMutex(_mutex);
}

void Mutex::unlock() {
	g_driver->unlockMutex(_mutex);
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
	g_driver->lockMutex(_mutex);
}

void StackLock::unlock() {
	g_driver->unlockMutex(_mutex);
}

}	// End of namespace Common
