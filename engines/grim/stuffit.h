/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/archive.h"
#include "common/scummsys.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"

#ifndef GRIM_STUFFIT_H
#define GRIM_STUFFIT_H

namespace Common {
class BitStream;
}

namespace Grim {

struct SIT14Data;

class StuffItArchive : public Common::Archive {
public:
	StuffItArchive();
	~StuffItArchive();

	bool open(const Common::String &filename);
	void close();
	bool isOpen() const { return _stream != 0; }

	// Common::Archive API implementation
	bool hasFile(const Common::String &name) const;
	int listMembers(Common::ArchiveMemberList &list) const;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;

private:
	struct FileEntry {
		byte compression;
		uint32 uncompressedSize;
		uint32 compressedSize;
		uint32 offset;
	};

	Common::SeekableReadStream *_stream;

	typedef Common::HashMap<Common::String, FileEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;
	FileMap _map;

	// Decompression Functions
	Common::SeekableReadStream *decompress14(Common::SeekableReadStream *src, uint32 uncompressedSize) const;

	// Decompression Helpers
	void update14(uint16 first, uint16 last, byte *code, uint16 *freq) const;
	void readTree14(Common::BitStream *bits, SIT14Data *dat, uint16 codesize, uint16 *result) const;
};

} // End of namespace Grim

#endif
