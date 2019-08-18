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

#ifndef BLADERUNNER_ARCHIVE_H
#define BLADERUNNER_ARCHIVE_H

#include "common/array.h"
#include "common/file.h"
#include "common/substream.h"

namespace BladeRunner {

class MIXArchive {
public:
	MIXArchive();
	~MIXArchive();

	static int32 getHash(const Common::String &name);
	static bool exists(const Common::String &filename);

	bool open(const Common::String &filename);
	void close();
	bool isOpen() const;

	Common::String getName() const { return _fd.getName(); }

	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name);

private:
	Common::File _fd;
	bool _isTLK;

	uint16 _entryCount;
	uint32 _size;

	struct ArchiveEntry {
		int32  hash;
		uint32 offset;
		uint32 length;
	};

	Common::Array<ArchiveEntry> _entries;

	uint32 indexForHash(int32 hash) const;
};


} // End of namespace BladeRunner

#endif
