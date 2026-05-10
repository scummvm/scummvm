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

#ifndef TINSEL_PSX_ARCHIVE_H
#define TINSEL_PSX_ARCHIVE_H

#include "common/archive.h"
#include "common/scummsys.h"
#include "common/hashmap.h"

namespace Tinsel {

// PSX Index / Data archive format
//
// Several PSX versions store all resource files in a single uncompressed
// data file. A separate index file contains the names, offsets, and sizes.
// Used in DW1 German, DW1 Japanese, and all DW2 versions.

class PsxArchive : public Common::Archive {
public:
	PsxArchive();
	~PsxArchive() override;

	bool open(const Common::Path &indexFilePath, const Common::Path &dataFilePath, uint16 tinselVersion);
	void close();

	// Common::Archive API implementation
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

private:
	struct FileEntry {
		uint32 size;
		uint32 offset;
	};

	Common::Path _dataFilePath;

	typedef Common::HashMap<Common::Path, FileEntry, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> FileMap;
	FileMap _map;
};

} // End of namespace Tinsel

#endif
