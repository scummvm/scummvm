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

#ifndef NANCY_IFF_H
#define NANCY_IFF_H

#include "common/array.h"

namespace Common {
struct IFFChunk;
class SeekableReadStream;
}

namespace Nancy {

class NancyEngine;
class ResourceManager;

class IFF {
	friend class ResourceManager;
private:
	IFF(Common::SeekableReadStream *stream);

public:
	~IFF();

	const byte *getChunk(uint32 id, uint &size, uint index = 0) const;
	Common::SeekableReadStream *getChunkStream(const Common::String &id, uint index = 0) const;

	void list(Common::Array<Common::String> &nameList) const;

private:
	static Common::String idToString(uint32 id);
	static uint32 stringToId(const Common::String &s);

	bool callback(Common::IFFChunk &chunk);

	struct Chunk {
		uint32 id;
		byte *buf;
		uint32 size;
	};

	const Common::SeekableReadStream *_stream;
	Common::Array<Chunk> _chunks;
	uint32 _nextDATAChunk;
};

} // End of namespace Nancy

#endif // NANCY_IFF_H
