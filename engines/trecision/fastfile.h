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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TRECISION_FASTFILE_H
#define TRECISION_FASTFILE_H

#include "common/archive.h"
#include "common/array.h"
#include "common/stream.h"

namespace Trecision {

class TrecisionEngine;

struct FileEntry {
	Common::String name;
	uint32 offset;
};

class FastFile : public Common::Archive {
public:
	FastFile();
	~FastFile() override;

	bool open(TrecisionEngine *vm, const Common::String &filename);
	void close();
	bool isOpen() const { return _stream != 0; }
	Common::SeekableReadStream *createReadStreamForCompressedMember(const Common::String &name);

	// Common::Archive API implementation
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

private:
	Common::SeekableReadStreamEndian *_stream;
	Common::SeekableReadStream *_compStream;
	Common::Array<FileEntry> _fileEntries;

	uint8 *_compBuffer;
	const FileEntry *getEntry(const Common::String &name) const;
	void decompress(const uint8 *src, uint32 srcSize, uint8 *dst, uint32 decompSize);
};

} // End of namespace Trecision

#endif
