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

#ifndef DREAMWEB_RNCA_ARCHIVE_H
#define DREAMWEB_RNCA_ARCHIVE_H

#include "common/archive.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace DreamWeb {
class RNCAArchive : public Common::MemcachingCaseInsensitiveArchive {
public:
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList&) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SharedArchiveContents readContentsForPath(const Common::String& translated) const override;

	static RNCAArchive* open(Common::SeekableReadStream *stream, DisposeAfterUse::Flag dispose = DisposeAfterUse::NO);
	
private:
	class RNCAFileDescriptor {
	private:
		Common::String _fileName;
		
		// Offset of the file contents relative to the beginning of RNCA stream
		uint32 _fileDataOffset;

	        RNCAFileDescriptor(const Common::String& filename, uint32 off) : _fileName(filename), _fileDataOffset(off) {}
		friend class RNCAArchive;
	public:
		// It's public for hashmap
		RNCAFileDescriptor() : _fileDataOffset(0) {}
	};

	typedef Common::HashMap<Common::String, RNCAFileDescriptor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

	RNCAArchive(FileMap files, Common::SeekableReadStream *stream, DisposeAfterUse::Flag dispose)
		: _files(files), _stream(stream, dispose) {
	}

	FileMap _files;
	Common::DisposablePtr<Common::SeekableReadStream> _stream;
};
}
#endif
