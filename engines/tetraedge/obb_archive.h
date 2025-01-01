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

#ifndef TETRAEDGE_OBB_ARCHIVE_H
#define TETRAEDGE_OBB_ARCHIVE_H

#include "common/archive.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Tetraedge {

class ObbArchive : public Common::Archive {
public:
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList&) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

	// Similar to FileDescriptionBin but in native-endian and native strings.
	struct FileDescriptor {
		FileDescriptor() : _fileOffset(0), _fileSize(0) {}
		FileDescriptor(uint32 offset,
			       uint32 size) : _fileOffset(offset), _fileSize(size) {}
		
		uint32 _fileOffset;
		uint32 _fileSize;
	};

    	typedef Common::HashMap<Common::String, FileDescriptor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

	static bool readFileMap(Common::SeekableReadStream &indexFile, FileMap &files);
	static ObbArchive* open(const Common::Path& obbName);

private:
	ObbArchive(const FileMap& files,
		   const Common::Path& obbName) : _files(files), _obbName(obbName) {}

	FileMap _files;
	Common::Path _obbName;
};
}

#endif
