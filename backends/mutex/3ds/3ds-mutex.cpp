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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "common/scummsys.h"

#if defined(__3DS__)

#include "backends/mutex/3ds/3ds-mutex.h"

#include <3ds.h>

/**
 * 3DS mutex manager
 */
class _3DSMutexInternal final : public Common::MutexInternal {
public:
	_3DSMutexInternal() { RecursiveLock_Init(&_mutex); }
	~_3DSMutexInternal() override {}

	bool lock() override { RecursiveLock_Lock(&_mutex); return true; }
	bool unlock() override { RecursiveLock_Unlock(&_mutex); return true; }

private:
	RecursiveLock _mutex;
};

Common::MutexInternal *create3DSMutexInternal() {
	return new _3DSMutexInternal();
}

#endif
