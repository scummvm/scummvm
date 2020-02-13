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

#ifndef ZVISION_ZFS_ARCHIVE_H
#define ZVISION_ZFS_ARCHIVE_H

#include "common/archive.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Common {
class String;
}

namespace ZVision {

struct ZfsHeader {
	uint32 magic;
	uint32 unknown1;
	uint32 maxNameLength;
	uint32 filesPerBlock;
	uint32 fileCount;
	uint8 xorKey[4];
	uint32 fileSectionOffset;
};

struct ZfsEntryHeader {
	Common::String name;
	uint32 offset;
	uint32 id;
	uint32 size;
	uint32 time;
	uint32 unknown;
};

typedef Common::HashMap<Common::String, ZfsEntryHeader *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ZfsEntryHeaderMap;

class ZfsArchive : public Common::Archive {
public:
	ZfsArchive(const Common::String &fileName);
	ZfsArchive(const Common::String &fileName, Common::SeekableReadStream *stream);
	~ZfsArchive() override;

	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	bool hasFile(const Common::String &fileName) const override;

	/**
	 * Add all members of the Archive to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return    The number of names added to list
	 */
	int listMembers(Common::ArchiveMemberList &list) const override;

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 *
	 * @return    The newly created input stream
	 */
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;

private:
	const Common::String _fileName;
	ZfsHeader _header;
	ZfsEntryHeaderMap _entryHeaders;

	/**
	 * Parses the zfs file into file entry headers that can be used later
	 * to get the entry data.
	 *
	 * @param stream    The contents of the zfs file
	 */
	void readHeaders(Common::SeekableReadStream *stream);

	/**
	 * Entry names are contained within a 16 byte block. This reads the block
	 * and converts it the name to a Common::String
	 *
	 * @param stream    The zfs file stream
	 * @return          The entry file name
	 */
	Common::String readEntryName(Common::SeekableReadStream *stream) const;

	/**
	 * ZFS file entries can be encrypted using XOR encoding. This method
	 * decodes the buffer in place using the supplied xorKey.
	 *
	 * @param buffer    The data to decode
	 * @param length    Length of buffer
	 */
	void unXor(byte *buffer, uint32 length, const byte *xorKey) const;
};

} // End of namespace ZVision

#endif
