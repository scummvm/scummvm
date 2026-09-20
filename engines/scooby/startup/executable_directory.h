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

#ifndef SCOOBY_EXECUTABLE_DIRECTORY_H
#define SCOOBY_EXECUTABLE_DIRECTORY_H

#include <string>

// Isolates the platform-specific "running executable's directory" lookup in its own
// translation unit. On Windows this requires <windows.h>, whose <winuser.h> portion declares
// CloseWindow and ShowCursor as extern "C" with signatures that collide with raylib.h's own
// CloseWindow/ShowCursor declarations (MSVC error C2733) if both headers are ever visible in the
// same translation unit. Keeping this lookup in a .cpp that never includes raylib.h (directly or
// transitively) sidesteps that collision entirely, so callers such as main.cpp -- which does
// include raylib.h through RaylibHost.h/ApplicationLoop.h -- only ever see this narrow
// declaration.

namespace Scooby {
namespace Startup {
// Approximates .NET AppContext.BaseDirectory: the directory containing the running executable,
// with a trailing path separator. Returns an empty string when the platform-specific lookup is
// unavailable or fails; callers then simply have one fewer candidate to try.
std::string GetExecutableDirectory();
} // namespace Startup
} // namespace Scooby

#endif // SCOOBY_EXECUTABLE_DIRECTORY_H
