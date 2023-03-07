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

#define ID_DATA		MKTAG('D', 'A', 'T', 'A')
#define ID_PCAL		MKTAG('P', 'C', 'A', 'L')

class IFF {
public:
	IFF(const Common::String &name) : _name(name) { };
	~IFF();

	bool load();
	const byte *getChunk(uint32 id, uint &size, uint index = 0) const;
	Common::SeekableReadStream *getChunkStream(const Common::String &id, uint index = 0) const;

	// Debugger functions
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

	Common::Array<Chunk> _chunks;
	const Common::String _name;
};

} // End of namespace Nancy

#endif // NANCY_IFF_H
