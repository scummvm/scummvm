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

#ifndef HYPNO_LIBFILE_H
#define HYPNO_LIBFILE_H

#include "common/archive.h"
#include "common/array.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/stream.h"

namespace Hypno {

typedef struct FileEntry {
	Common::String name;
	uint32 start;
	uint32 size;
} FileEntry;

class LibFile : public Common::Archive {
public:
	LibFile();
	~LibFile() override;

	bool open(const Common::String &prefix, const Common::String &filename, bool encrypted);
	void close();

	// Common::Archive API implementation
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

private:
	bool _encrypted;
	Common::File *_libfile;
	Common::String _prefix;
	Common::Array<FileEntry> _fileEntries;
	const FileEntry *getEntry(const Common::Path &path) const;
};

} // End of namespace Hypno

#endif

