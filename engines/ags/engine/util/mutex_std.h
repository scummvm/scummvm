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

#ifndef AGS_ENGINE_UTIL_MUTEX_STD_H
#define AGS_ENGINE_UTIL_MUTEX_STD_H

#include "ags/lib/std/mutex.h"
#include "ags/engine/util/mutex.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

class StdMutex : public BaseMutex {
public:
	inline StdMutex() : mutex_() {
	}
	inline ~StdMutex() override {}

	StdMutex &operator=(const StdMutex &) = delete;
	StdMutex(const StdMutex &) = delete;

	inline void Lock() override {
		mutex_.lock();
	}
	inline void Unlock() override {
		mutex_.unlock();
	}

private:
	std::recursive_mutex mutex_;
};

typedef StdMutex Mutex;

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
