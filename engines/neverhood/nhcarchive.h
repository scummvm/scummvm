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

#ifndef NEVERHOOD_NHCARCHIVE_H
#define NEVERHOOD_NHCARCHIVE_H

#include "common/array.h"
#include "common/file.h"
#include "common/mutex.h"
#include "common/stream.h"
#include "common/substream.h"
#include "neverhood/neverhood.h"

namespace Neverhood {

struct NhcArchiveEntry {
	uint32 fileHash;
	uint32 type;
	uint32 offset;
	uint32 size;

	bool isNormal() const {
		// Resources 0-10 replace resources with the same ID
		// Resources 11 and 12 are under custom ID for subtitle font and saveload dialog messages
		// 13 adds subtitles to a video with the same ID and so should not replace it
		return type <= 10;
	}
};

class NhcArchive {
public:
	NhcArchive() {}
	~NhcArchive() {}
	bool open(const Common::String &filename, bool isOptional);
	void load(uint index, byte *buffer, uint32 size);
	void load(NhcArchiveEntry *entry, byte *buffer, uint32 size);
	uint32 getSize(uint index) { return _entries[index].size; }
	NhcArchiveEntry *getEntry(uint index) { return &_entries[index]; }
	uint getCount() { return _entries.size(); }
	Common::SeekableReadStream *createStream(uint index);
	Common::SeekableReadStream *createStream(NhcArchiveEntry *entry);
private:
	Common::File _fd;
	Common::Mutex _mutex;
	Common::Array<NhcArchiveEntry> _entries;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_BLBARCHIVE_H */
