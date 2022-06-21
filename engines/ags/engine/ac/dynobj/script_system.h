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

// ScriptSystem is a readable/writeable struct which has been exposed to
// script in older versions of API (deprecated).
// WARNING: it *MUST* keep its size exact to avoid breaking address offsets
// when running old scripts. In case of emergency you may use its reserved
// fields, but it's not recommended to do, as this struct is not a part of
// the modern API anymore.

#ifndef AGS_ENGINE_DYNOBJ_SCRIPT_SYSTEM_H
#define AGS_ENGINE_DYNOBJ_SCRIPT_SYSTEM_H

namespace AGS3 {

// The text script's "system" struct
struct ScriptSystem {
	int width = 0; // game screen width
	int height = 0; // game screen height
	int coldepth = 0; // game's color depth
	int os = 0; // operating system's code (see eScriptSystemOSID)
	int windowed = 0; // windowed/fullscreen flag
	int vsync = 0; // vertical sync flag
	int viewport_width = 0; // game viewport width (normal or letterboxed)
	int viewport_height = 0; // game viewport height (normal or letterboxed)
	char aci_version[10]{}; // engine version string (informational)
	int reserved[5]{}; // reserved fields
};

} // namespace AGS3

#endif
