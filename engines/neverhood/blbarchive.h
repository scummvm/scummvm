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

#ifndef NEVERHOOD_BLBARCHIVE_H
#define NEVERHOOD_BLBARCHIVE_H

#include "common/array.h"
#include "common/file.h"
#include "common/mutex.h"
#include "common/stream.h"
#include "common/substream.h"
#include "neverhood/neverhood.h"

namespace Neverhood {

struct BlbHeader {
	uint32 id1;
	uint16 id2;
	uint16 extDataSize;
	int32 fileSize;
	uint32 fileCount;
};

struct BlbArchiveEntry {
	uint32 fileHash;
	byte type;
	byte comprType;
	byte *extData;
	uint32 timeStamp;
	uint32 offset;
	uint32 diskSize;
	uint32 size;
};

class BlbArchive {
public:
	BlbArchive();
	~BlbArchive();
	void open(const Common::String &filename);
	void load(uint index, byte *buffer, uint32 size);
	void load(BlbArchiveEntry *entry, byte *buffer, uint32 size);
	byte *getEntryExtData(uint index);
	byte *getEntryExtData(BlbArchiveEntry *entry);
	uint32 getSize(uint index) { return _entries[index].size; }
	BlbArchiveEntry *getEntry(uint index) { return &_entries[index]; }
	uint getCount() { return _entries.size(); }
	Common::SeekableReadStream *createStream(uint index);
	Common::SeekableReadStream *createStream(BlbArchiveEntry *entry);
private:
	Common::File _fd;
	Common::Mutex _mutex;
	Common::Array<BlbArchiveEntry> _entries;
	byte *_extData;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_BLBARCHIVE_H */
