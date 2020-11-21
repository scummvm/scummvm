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

#ifndef AGS_ENGINE_UTIL_THREAD_WII_H
#define AGS_ENGINE_UTIL_THREAD_WII_H

#include <gccore.h>

namespace AGS {
namespace Engine {


class WiiThread : public BaseThread {
public:
	WiiThread() {
		_thread = (lwp_t)NULL;
		_entry = NULL;
		_running = false;
		_looping = false;
	}

	~WiiThread() {
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
			_running = (LWP_CreateThread(&_thread, _thread_start, this, 0, 8 * 1024, 64) != 0);

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

			LWP_JoinThread(_thread, NULL);

			_running = false;
			return true;
		} else {
			return false;
		}
	}

private:
	lwp_t     _thread;
	bool      _running;
	bool      _looping;

	AGSThreadEntry _entry;

	static void *_thread_start(void *arg) {
		AGSThreadEntry entry = ((WiiThread *)arg)->_entry;
		bool *looping = &((WiiThread *)arg)->_looping;

		do {
			entry();
		} while (*looping);

		return NULL;
	}
};


typedef WiiThread Thread;


} // namespace Engine
} // namespace AGS

#endif
