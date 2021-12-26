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

#ifndef AGS_ENGINE_UTIL_MUTEX_LOCK_H
#define AGS_ENGINE_UTIL_MUTEX_LOCK_H

#include "ags/engine/util/mutex.h"

namespace AGS3 {
namespace AGS {
namespace Engine {


class MutexLock {
private:
	BaseMutex *_m;
	MutexLock(MutexLock const &); // non-copyable
	MutexLock &operator=(MutexLock const &); // not copy-assignable

public:
	void Release() {
		if (_m != nullptr) _m->Unlock();
		_m = nullptr;
	}

	void Acquire(BaseMutex &mutex) {
		Release();
		_m = &mutex;
		_m->Lock();
	}

	MutexLock() : _m(nullptr) {
	}

	explicit MutexLock(BaseMutex &mutex) : _m(nullptr) {
		Acquire(mutex);
	}

	~MutexLock() {
		Release();
	}
}; // class MutexLock


} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
