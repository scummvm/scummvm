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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(__WII__)

#include "backends/mutex/wii/wii-mutex.h"

#include <ogc/mutex.h>

/**
 * Wii mutex implementation
 */
class WiiMutexInternal final : public Common::MutexInternal {
public:
	WiiMutexInternal();
	~WiiMutexInternal() override;

	bool lock() override;
	bool unlock() override;

private:
	mutex_t _mutex;
};


WiiMutexInternal::WiiMutexInternal() {
	s32 res = LWP_MutexInit(&_mutex, true);

	if (res) {
		printf("ERROR creating mutex\n");
	}
}

WiiMutexInternal::~WiiMutexInternal() {
	s32 res = LWP_MutexDestroy(_mutex);

	if (res)
		printf("ERROR destroying mutex (%d)\n", res);
}

bool WiiMutexInternal::lock() {
	s32 res = LWP_MutexLock(_mutex);

	if (res) {
		printf("ERROR locking mutex (%d)\n", res);
		return false;
	} else {
		return true;
	}
}

bool WiiMutexInternal::unlock() {
	s32 res = LWP_MutexUnlock(_mutex);

	if (res) {
		printf("ERROR unlocking mutex (%d)\n", res);
		return false;
	} else {
		return true;
	}
}

Common::MutexInternal *createWiiMutexInternal() {
	return new WiiMutexInternal();
}

#endif
