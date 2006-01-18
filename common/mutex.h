/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef COMMON_MUTEX_H
#define COMMON_MUTEX_H

#include "common/scummsys.h"

namespace Common {

class Mutex;

/**
 * An pseudo-opaque mutex type. See OSystem::createMutex etc. for more details.
 */
typedef struct OpaqueMutex *MutexRef;


/**
 * Auxillary class to (un)lock a mutex on the stack.
 */
class StackLock {
	MutexRef _mutex;
	const char *_mutexName;

	void lock();
	void unlock();
public:
	StackLock(MutexRef mutex, const char *mutexName = NULL);
	StackLock(const Mutex &mutex, const char *mutexName = NULL);
	~StackLock();
};


/**
 * Wrapper class around the OSystem mutex functions.
 */
class Mutex {
	friend class StackLock;

	MutexRef _mutex;

public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();
};


} // End of namespace Common

#endif
