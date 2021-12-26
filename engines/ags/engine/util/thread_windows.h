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

#ifndef AGS_ENGINE_UTIL_THREAD_WINDOWS_H
#define AGS_ENGINE_UTIL_THREAD_WINDOWS_H

namespace AGS3 {
namespace AGS {
namespace Engine {


class WindowsThread : public BaseThread {
public:
	WindowsThread() {
		_thread = NULL;
		_entry = NULL;
		_running = false;
		_looping = false;
	}

	~WindowsThread() {
		Stop();
	}

	inline bool Create(AGSThreadEntry entryPoint, bool looping) {
		_looping = looping;
		_entry = entryPoint;
		_thread = CreateThread(NULL, 0, _thread_start, this, CREATE_SUSPENDED, NULL);

		return (_thread != NULL);
	}

	inline bool Start() {
		if ((_thread != NULL) && (!_running)) {
			DWORD result = ResumeThread(_thread);

			_running = (result != (DWORD) - 1);
			return _running;
		} else {
			return false;
		}
	}

	bool Stop() {
		if ((_thread != NULL) && (_running)) {
			if (_looping) {
				_looping = false;
				WaitForSingleObject(_thread, INFINITE);
			}

			CloseHandle(_thread);

			_running = false;
			_thread = NULL;
			return true;
		} else {
			return false;
		}
	}

private:
	HANDLE _thread;
	bool   _running;
	bool   _looping;

	AGSThreadEntry _entry;

	static DWORD __stdcall _thread_start(LPVOID lpParam) {
		AGSThreadEntry entry = ((WindowsThread *)lpParam)->_entry;
		bool *looping = &((WindowsThread *)lpParam)->_looping;

		do {
			entry();
		} while (*looping);

		return 0;
	}
};


typedef WindowsThread Thread;


} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
