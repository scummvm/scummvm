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

#ifndef MADS_RESOURCES_H
#define MADS_RESOURCES_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/serializer.h"
#include "common/str.h"

namespace MADS {

class MADSEngine;

enum RESPREFIX {
	RESPREFIX_GL = 1, RESPREFIX_SC = 2, RESPREFIX_RM = 3
};

enum EXTTYPE {
	EXT_NONE = -1, EXT_SS = 1, EXT_AA = 2, EXT_DAT = 3, EXT_HH = 4,
	EXT_ART = 5, EXT_INT = 6
};

class Resources {
public:
	/**
	 * Instantiates the resource manager
	 */
	static void init(MADSEngine *vm);

	static Common::String formatName(RESPREFIX resType, int id, const Common::String &ext);
	static Common::String formatName(int prefix, char asciiCh, int id,
		EXTTYPE extType, const Common::String &suffix);
	static Common::String formatResource(const Common::String &resName, const Common::String &hagFilename);
	static Common::String formatAAName(int idx);
};

/**
 * Derived file class
 */
class File : public Common::File {
public:
	/**
	 * Constructor
	 */
	File() : Common::File() {}

	/**
	 * Constructor
	 */
	File(const Common::String &filename) { openFile(filename); }

	/**
	 * Opens the given file, throwing an error if it can't be opened
	 */
	void openFile(const Common::String &filename);
};

class SynchronizedList : public Common::Array<int> {
public:
	/**
	 * Synchronize the list
	 */
	void synchronize(Common::Serializer &s);
};

} // End of namespace MADS

#endif /* MADS_RESOURCES_H */
