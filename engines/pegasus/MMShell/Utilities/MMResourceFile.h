/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PEGASUS_MMSHELL_UTILITIES_MMRESOURCEFILE
#define PEGASUS_MMSHELL_UTILITIES_MMRESOURCEFILE

#include "common/str.h"

#include "pegasus/MMShell/MMTypes.h"

namespace Common {
	class Error;
	class MacResManager;
	class SeekableReadStream;
}

namespace Pegasus {

class MMResourceFile {
public:
	MMResourceFile();
	virtual ~MMResourceFile();

	Common::Error UsePathname(const Common::String &str);

	// For ScummVM, we don't need this function
	//OSErr MMResourceFile::UseFSSpec(const FSSpec&);
	
	virtual Common::Error OpenResourceFile();
	virtual Common::Error CloseResourceFile();
	bool IsFileOpen() const;
	
	virtual Common::Error GetResource(const uint32 type, const tResIDType id, Common::SeekableReadStream *&h);
	virtual Common::Error GetResource(const uint32 type, const Common::String &name, Common::SeekableReadStream *&h);

	// This function is unused!
	//virtual OSErr PutResource(const ResType, const tResIDType, const Common::String &, Handle);

	// For ScummVM, we will be hardcoding the file names so these two functions
	// do nothing for us
	//virtual OSErr GetIndString(const tResIDType, const short, Str255&);
	//virtual OSErr GetIndString(const Str255, const short, Str255&);

	virtual Common::Error GetCIcon(const tResIDType id, Common::SeekableReadStream *&icon);

protected:
	Common::MacResManager *_resFork;
	Common::String _fileName;
};

} // End of namespace Pegasus

#endif