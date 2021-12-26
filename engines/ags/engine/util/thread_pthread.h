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

#ifndef AGS_ENGINE_PLATFORM_THREAD_PTHREAD_H
#define AGS_ENGINE_PLATFORM_THREAD_PTHREAD_H

//include <pthread.h>

namespace AGS3 {
namespace AGS {
namespace Engine {


class PThreadThread : public BaseThread {
public:
	PThreadThread() {
		_thread = 0;
		_entry = NULL;
		_running = false;
		_looping = false;
	}

	~PThreadThread() {
		Stop();
	}

	inline bool Create(AGSThreadEntry entryPoint, bool looping) {
		_looping = looping;
		_entry = entryPoint;

		// Thread creation is delayed till the thread is started
		return true;
	}

	inline bool Start() {
		if (!_running) {
			_running = (pthread_create(&_thread, NULL, _thread_start, this) == 0);

			return _running;
		} else {
			return false;
		}
	}

	bool Stop() {
		if (_running) {
			if (_looping) {
				_looping = false;
			}

			pthread_join(_thread, NULL);

			_running = false;
			return true;
		} else {
			return false;
		}
	}

private:
	pthread_t _thread;
	bool      _running;
	bool      _looping;

	AGSThreadEntry _entry;

	static void *_thread_start(void *arg) {
		AGSThreadEntry entry = ((PThreadThread *)arg)->_entry;
		bool *looping = &((PThreadThread *)arg)->_looping;

		do {
			entry();
		} while (*looping);

		return NULL;
	}
};


typedef PThreadThread Thread;


} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
