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

#ifndef AGS_ENGINE_UTIL_WINDOWS_MUTEX_H
#define AGS_ENGINE_UTIL_WINDOWS_MUTEX_H

namespace AGS3 {
namespace AGS {
namespace Engine {


class WindowsMutex : public BaseMutex {
public:
	WindowsMutex() {
		_mutex = CreateMutex(NULL, FALSE, NULL);

		_ASSERT(_mutex != NULL);
	}

	~WindowsMutex() {
		_ASSERT(_mutex != NULL);

		CloseHandle(_mutex);
	}

	inline void Lock() {
		_ASSERT(_mutex != NULL);

		WaitForSingleObject(_mutex, INFINITE);
	}

	inline void Unlock() {
		_ASSERT(_mutex != NULL);

		ReleaseMutex(_mutex);
	}

private:
	HANDLE _mutex;
};


typedef WindowsMutex Mutex;

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
