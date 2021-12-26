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

#include "common/scummsys.h"

#if defined(SDL_BACKEND)

#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/platform/sdl/sdl-sys.h"

/**
 * SDL mutex manager
 */
class SdlMutexInternal final : public Common::MutexInternal {
public:
	SdlMutexInternal() { _mutex = SDL_CreateMutex(); }
	~SdlMutexInternal() override { SDL_DestroyMutex(_mutex); }

	bool lock() override { return (SDL_mutexP(_mutex) == 0); }
	bool unlock() override { return (SDL_mutexV(_mutex) == 0); }

private:
	SDL_mutex *_mutex;
};

Common::MutexInternal *createSdlMutexInternal() {
	return new SdlMutexInternal();
}

#endif
