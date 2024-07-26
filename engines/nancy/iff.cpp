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

IFF::IFF(Common::SeekableReadStream *stream) {
	// Scan the file for FORM/DATA wrapper chunks. There can be several of these in a single IFF.
	uint32 dataString = g_nancy->getGameType() == kGameTypeVampire ? ID_FORM : ID_DATA;
	_stream = stream;

	while (stream->pos() < stream->size() - 3) {
		_nextDATAChunk = 0;
		uint32 id = stream->readUint32BE();
		stream->seek(-4, SEEK_CUR);
		if (id == dataString) {
			Common::IFFParser iff(stream, false, dataString);
			Common::Functor1Mem<Common::IFFChunk &, bool, IFF> c(this, &IFF::callback);
			iff.parse(c);
			if (_nextDATAChunk) {
				stream->seek(_nextDATAChunk);
			}
		} else {
			stream->skip(1);
		}
	}

	delete _stream;
	_stream = nullptr;
}

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

	if (chunk.id == (g_nancy->getGameType() == kGameTypeVampire ? ID_FORM : ID_DATA)) {
		// Encountered the next FORM/DATA wrapper. Signal that we need to stop reading the
		// current one and mark where from the parser should be called next.
		_nextDATAChunk = _stream->pos() - 8;
		return true;
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
		byte *dup = (byte *)malloc(size);
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
	Common::String chunkName;
	nameList.reserve(_chunks.size());
	for (uint i = 0; i < _chunks.size(); ++i) {
		chunkName = idToString(_chunks[i].id);
		chunkName.trim();
		nameList.push_back(chunkName);
	}
}

} // End of namespace Nancy
