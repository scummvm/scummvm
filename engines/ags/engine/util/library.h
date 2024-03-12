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

#ifndef AGS_ENGINE_UTIL_LIBRARY_H
#define AGS_ENGINE_UTIL_LIBRARY_H

#include "ags/shared/core/platform.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

using AGS::Shared::String;

class BaseLibrary {
public:
	BaseLibrary() {}

	virtual ~BaseLibrary() {}

	// Get library name; returns empty string if not loaded
	inline String GetName() const { return _name; }
	// Get actual filename; returns empty string if not loaded
	inline String GetFileName() const { return _filename; }
	// Get path used to load the library; or empty string is not loaded.
	// NOTE: this is NOT a fully qualified path, but a lookup path.
	inline String GetPath() const { return _path; }

	// Returns expected filename form for the dynamic library of a given name
	virtual String GetFilenameForLib(const String &libname) = 0;

	// Try load a library of a given name
	virtual bool Load(const String &libname) = 0;
	// Unload a library; does nothing if was not loaded
	virtual void Unload() = 0;
	// Tells if library is loaded
	virtual bool IsLoaded() const = 0;

protected:
	String _name;
	String _filename;
	String _path;
};

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
