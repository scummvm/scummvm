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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef XEEN_FILES_H
#define XEEN_FILES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/file.h"
#include "graphics/surface.h"
#include "xeen/xsurface.h"

namespace Xeen {

class XeenEngine;

/*
 * Main resource manager
 */
class FileManager {
public:
	static void init(XeenEngine *vm);
};

/**
 * Derived file class
 */
class File : public Common::File {
public:
	File() : Common::File() {}
	File(const Common::String &filename) { openFile(filename); }
	virtual ~File() {}

	void openFile(const Common::String &filename);
};

/**
* Xeen CC file implementation
*/
class CCArchive : public Common::Archive {
private:
	/**
	* Details of a single entry in a CC file index
	*/
	struct CCEntry {
		uint16 _id;
		uint32 _offset;
		uint16 _size;

		CCEntry() : _id(0), _offset(0), _size(0) {}
		CCEntry(uint16 id, uint32 offset, uint32 size)
			: _id(id), _offset(offset), _size(size) {
		}
	};

	Common::Array<CCEntry> _index;
	Common::String _filename;
	bool _encoded;

	uint16 convertNameToId(const Common::String &resourceName) const;

	void loadIndex(Common::SeekableReadStream *stream);

	bool getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const;
public:
	CCArchive(const Common::String &filename, bool encoded = true);
	virtual ~CCArchive();

	// Archive implementation
	virtual bool hasFile(const Common::String &name) const;
	virtual int listMembers(Common::ArchiveMemberList &list) const;
	virtual const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;
};

} // End of namespace Xeen

#endif /* XEEN_FILES_H */
