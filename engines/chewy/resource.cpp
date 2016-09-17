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

#include "common/stream.h"
#include "common/debug.h"
#include "common/textconsole.h"

#include "chewy/chewy.h"
#include "chewy/resource.h"

namespace Chewy {

Resource::Resource(Common::String filename) {
	_stream.open(filename);
	uint32 magicBytes = MKTAG('N', 'G', 'S', '\0');
	if (_stream.readUint32BE() != magicBytes)
		error("Invalid resource - %s", filename.c_str());
	_stream.skip(2);	// type
	_chunkCount = _stream.readUint16LE();

	uint32 tbfID = MKTAG('T', 'B', 'F', '\0');

	for (uint i = 0; i < _chunkCount; i++) {
		TBFChunk cur;
		cur.packedSize = _stream.readUint32LE();
		cur.type = _stream.readUint16LE();
		cur.pos = _stream.pos();
		if (_stream.readUint32BE() != tbfID)
			error("Corrupt resource %s", filename.c_str());

		cur.screenMode = _stream.readUint16LE();
		cur.compressionFlag = _stream.readUint16LE();
		cur.unpackedSize = _stream.readUint32LE();
		cur.width = _stream.readUint16LE();
		cur.height = _stream.readUint16LE();
		_stream.read(cur.palette, 3 * 256);
		_stream.skip(cur.packedSize - TBF_CHUNK_HEADER_SIZE);

		_tbfChunkList.push_back(cur);

		/*debug("Chunk %d: packed %d, type %d, pos %d, mode %d, comp %d, unpacked %d, width %d, height %d",
			i, cur.packedSize, cur.type, cur.pos, cur.screenMode, cur.compressionFlag, cur.unpackedSize,
			cur.width, cur.height
		);*/
	}

}

Resource::~Resource() {
	_tbfChunkList.clear();
	_stream.close();
}

TBFChunk *Resource::getChunk(int num) {
	return &_tbfChunkList[num];
}

Common::SeekableReadStream *Resource::getChunkData(int num) {
	// TODO
	return nullptr;
}

} // End of namespace Chewy
