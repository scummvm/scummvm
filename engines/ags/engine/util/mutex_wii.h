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

#ifndef AGS_ENGINE_UTIL_WII_MUTEX_H
#define AGS_ENGINE_UTIL_WII_MUTEX_H

//include <gccore.h>

namespace AGS3 {
namespace AGS {
namespace Engine {

class WiiMutex : public BaseMutex {
public:
	inline WiiMutex() {
		LWP_MutexInit(&_mutex, 0);
	}

	inline ~WiiMutex() {
		LWP_MutexDestroy(_mutex);
	}

	inline void Lock() {
		LWP_MutexLock(_mutex);
	}

	inline void Unlock() {
		LWP_MutexUnlock(_mutex);
	}

private:
	mutex_t _mutex;
};

typedef WiiMutex Mutex;

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
