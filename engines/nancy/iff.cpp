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

#include "common/memstream.h"
#include "common/formats/iff_container.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/resource.h"

namespace Nancy {

IFF::~IFF() {
	for (uint i = 0; i < _chunks.size(); i++)
		delete[] _chunks[i].buf;
}

bool IFF::callback(Common::IFFChunk &c) {
	Chunk chunk;

	// Replace invalid NULs with spaces
	char id[4];
	WRITE_BE_UINT32(id, c._type);
	for (uint i = 0; i < 4; ++i) {
		if (id[i] == 0)
			id[i] = ' ';
	}
	chunk.id = READ_BE_UINT32(id);

	if (chunk.id == ID_DATA) {
		debugN(3, "IFF::callback: Skipping 'DATA' chunk\n");
		return false;
	}

	chunk.size = c._size;
	chunk.buf = new byte[chunk.size];
	c._stream->read(chunk.buf, chunk.size);

	if (c._stream->err())
		error("IFF::callback: error reading '%s' chunk", idToString(chunk.id).c_str());

	debugN(3, "IFF::callback: Adding '%s' chunk\n", idToString(chunk.id).c_str());
	_chunks.push_back(chunk);

	return false;
}

bool IFF::load() {
	byte *data;
	uint size;
	data = g_nancy->_resource->loadData(_name, size);

	if (!data) {
		return false;
	}

	// Scan the file for DATA chunks, completely ignoring IFF structure
	// Presumably the string "DATA" is not allowed inside of chunks...
	// The Vampire Diaries uses the standard FORM
	uint32 dataString = g_nancy->getGameType() == kGameTypeVampire ? ID_FORM : ID_DATA;

	uint offset = 0;

	while (offset < size - 3) {
		uint32 id = READ_BE_UINT32(data + offset);
		if (id == dataString) {
			// Replace 'DATA' with standard 'FORM' for the parser
			WRITE_BE_UINT32(data + offset, ID_FORM);
			Common::MemoryReadStream stream(data + offset, size - offset);
			Common::IFFParser iff(&stream);
			Common::Functor1Mem<Common::IFFChunk &, bool, IFF> c(this, &IFF::callback);
			iff.parse(c);
			offset += 16; // Original engine skips 16, while 12 seems more logical
		} else {
			++offset;
		}
	}

	delete[] data;
	return true;
}

const byte *IFF::getChunk(uint32 id, uint &size, uint index) const {
	uint found = 0;
	for (uint i = 0; i < _chunks.size(); ++i) {
		const Chunk &chunk = _chunks[i];
		if (chunk.id == id) {
			if (found == index) {
				size = chunk.size;
				return chunk.buf;
			}
			++found;
		}
	}

	return nullptr;
}

Common::SeekableReadStream *IFF::getChunkStream(const Common::String &id, uint index) const {
	uint size;
	const byte *chunk = getChunk(stringToId(id), size, index);

	if (chunk) {
		byte *dup = new byte[size];
		memcpy(dup, chunk, size);
		return new Common::MemoryReadStream(dup, size, DisposeAfterUse::YES);
	}

	return nullptr;
}

Common::String IFF::idToString(uint32 id) {
	Common::String s;
	while (id) {
		s += id >> 24;
		id <<= 8;
	}
	return s;
}

uint32 IFF::stringToId(const Common::String &s) {
	uint32 id = 0;

	for (uint i = 0; i < 4; ++i)
		id |= (s.size() > i ? s[i] : ' ') << (3 - i) * 8;

	return id;
}

void IFF::list(Common::Array<Common::String> &nameList) const {
	nameList.reserve(_chunks.size());
	for (uint i = 0; i < _chunks.size(); ++i) {
		nameList.push_back(idToString(_chunks[i].id));
	}
}

} // End of namespace Nancy
