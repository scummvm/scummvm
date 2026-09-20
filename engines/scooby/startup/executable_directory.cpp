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

#include "executable_directory.h"

// This translation unit deliberately includes no raylib header (see ExecutableDirectory.h for
// why): <windows.h> here would otherwise collide with raylib.h's CloseWindow/ShowCursor
// declarations under MSVC.

#if defined(__linux__)

#include <unistd.h>
#elif defined(__APPLE__)
#include "common/scummsys.h"
#include <mach-o/dyld.h>
#include <vector>
#elif defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace Scooby {
namespace Startup {

std::string GetExecutableDirectory() {
#if defined(__linux__)
	char buffer[4096];
	ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
	if (length <= 0) {
		return std::string();
	}

	std::string path(buffer, static_cast<std::size_t>(length));
	std::string::size_type lastSlash = path.find_last_of('/');
	if (lastSlash == std::string::npos) {
		return std::string();
	}

	return path.substr(0, lastSlash + 1);
#elif defined(__APPLE__)
	uint32 size = 0;
	_NSGetExecutablePath(nullptr, &size);
	std::vector<char> buffer(size);
	if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
		return std::string();
	}

	std::string path(buffer.data());
	std::string::size_type lastSlash = path.find_last_of('/');
	if (lastSlash == std::string::npos) {
		return std::string();
	}

	return path.substr(0, lastSlash + 1);
#elif defined(_WIN32)
	char buffer[MAX_PATH];
	DWORD length = GetModuleFileNameA(nullptr, buffer, static_cast<DWORD>(sizeof(buffer)));
	if (length == 0 || length == sizeof(buffer)) {
		return std::string();
	}

	std::string path(buffer, length);
	std::string::size_type lastSlash = path.find_last_of("\\/");
	if (lastSlash == std::string::npos) {
		return std::string();
	}

	return path.substr(0, lastSlash + 1);
#else
	return std::string();
#endif
}
} // namespace Startup
} // namespace Scooby