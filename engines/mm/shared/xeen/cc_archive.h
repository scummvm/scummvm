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

#ifndef MM_SHARED_XEEN_CC_ARCHIVE_H
#define MM_SHARED_XEEN_CC_ARCHIVE_H

#include "common/archive.h"
#include "common/stream.h"

namespace MM {
namespace Shared {
namespace Xeen {

/**
 * Details of a single entry in a CC file index
 */
struct CCEntry {
	uint16 _id;
	int _offset;
	uint16 _size;
	int _writeOffset;

	CCEntry() : _id(0), _offset(0), _size(0), _writeOffset(0) {
	}
	CCEntry(uint16 id, uint32 offset, uint32 size)
		: _id(id), _offset(offset), _size(size) {
	}
};

/**
 * Base Xeen CC file implementation
 */
class BaseCCArchive : public Common::Archive {
protected:
	Common::Array<CCEntry> _index;

	/**
	 * Load the index of a given CC file
	 */
	void loadIndex(Common::SeekableReadStream &stream);

	/**
	 * Saves out the contents of the index. Used when creating savegames
	 */
	void saveIndex(Common::WriteStream &stream);

	/**
	 * Given a resource name, returns whether an entry exists, and returns
	 * the header index data for that entry
	 */
	virtual bool getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const;

	/**
	 * Given a resource Id, returns whether an entry exists, and returns
	 * the header index data for that entry
	 */
	virtual bool getHeaderEntry(uint16 id, CCEntry &ccEntry) const;
public:
	/**
	 * Hash a given filename to produce the Id that represents it
	 */
	static uint16 convertNameToId(const Common::String &resourceName);
public:
	BaseCCArchive() {
	}

	// Archive implementation
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
};

/**
 * Xeen CC file implementation
 */
class CCArchive : public BaseCCArchive {
private:
	Common::String _filename;
	Common::String _prefix;
	bool _encoded;
protected:
	bool getHeaderEntry(const Common::String &resourceName, CCEntry &ccEntry) const override;
public:
	CCArchive(const Common::String &filename, bool encoded);
	CCArchive(const Common::String &filename, const Common::String &prefix, bool encoded);
	~CCArchive() override;

	// Archive implementation
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
};

} // namespace Xeen
} // namespace Shared
} // namespace MM

#endif
