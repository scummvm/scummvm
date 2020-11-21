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

#ifndef AGS_ENGINE_UTIL_PSP_THREAD_H
#define AGS_ENGINE_UTIL_PSP_THREAD_H

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman.h>

namespace AGS {
namespace Engine {


class PSPThread : public BaseThread {
public:
	PSPThread() {
		_thread = -1;
		_entry = NULL;
		_running = false;
		_looping = false;
	}

	~PSPThread() {
		Stop();
	}

	inline bool Create(AGSThreadEntry entryPoint, bool looping) {
		_looping = looping;
		_entry = entryPoint;
		_thread = sceKernelCreateThread("ags", _thread_start, 0x20, 0x10000, THREAD_ATTR_USER, 0);

		return (_thread > -1);
	}

	inline bool Start() {
		if ((_thread > -1) && (!_running)) {
			PSPThread *thisPointer = this;
			SceUID result = sceKernelStartThread(_thread, sizeof(this), &thisPointer);

			_running = (result > -1);
			return _running;
		} else {
			return false;
		}
	}

	bool Stop() {
		if ((_thread > -1) && (_running)) {
			if (_looping) {
				_looping = false;
				sceKernelWaitThreadEnd(_thread, 0);
			}

			_running = false;
			return (sceKernelTerminateDeleteThread(_thread) > -1);
		} else {
			return false;
		}
	}

private:
	SceUID _thread;
	bool   _running;
	bool   _looping;

	AGSThreadEntry _entry;

	static int _thread_start(SceSize args, void *argp) {
		AGSThreadEntry entry = (*(PSPThread **)argp)->_entry;
		bool *looping = &(*(PSPThread **)argp)->_looping;

		do {
			entry();
		} while (*looping);
	}
};


typedef PSPThread Thread;


} // namespace Engine
} // namespace AGS

#endif
