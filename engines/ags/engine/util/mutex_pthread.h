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

#ifndef AGS_ENGINE_UTIL_MUTEX_PTHREAD_H
#define AGS_ENGINE_UTIL_MUTEX_PTHREAD_H

//include <pthread.h>

namespace AGS3 {
namespace AGS {
namespace Engine {


class PThreadMutex : public BaseMutex {
public:
	inline PThreadMutex() {
		pthread_mutex_init(&_mutex, NULL);
	}

	inline ~PThreadMutex() {
		pthread_mutex_destroy(&_mutex);
	}

	inline void Lock() {
		pthread_mutex_lock(&_mutex);
	}

	inline void Unlock() {
		pthread_mutex_unlock(&_mutex);
	}

private:
	pthread_mutex_t _mutex;
};

typedef PThreadMutex Mutex;


} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
