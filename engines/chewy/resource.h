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

#ifndef CHEWY_RESOURCE_H
#define CHEWY_RESOURCE_H


#include "common/scummsys.h"
#include "common/file.h"
#include "common/util.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/random.h"

namespace Chewy {

// 4 + 2 + 2 + 4 + 2 + 2 + 768 = 784 bytes
#define TBF_CHUNK_HEADER_SIZE 784

struct TBFChunk {
	uint32 packedSize;	// includes header
	uint16 type;

	// TBF chunk header
	// ID (TBF, followed by a zero)
	uint16 screenMode;
	uint16 compressionFlag;
	uint32 unpackedSize;
	uint16 width;
	uint16 height;
	byte palette[3 * 256];

	uint32 pos;	// position of the actual data
};

typedef Common::Array<TBFChunk> TBFChunkList;

class Resource {
public:
	Resource(Common::String filename);
	~Resource();

	TBFChunk *getChunk(int num);
	byte *getChunkData(int num);

private:
	Common::File _stream;
	uint16 _chunkCount;
	TBFChunkList _tbfChunkList;
};

} // End of namespace Chewy

#endif
