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

#include "common/debug.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "chewy/chewy.h"
#include "chewy/resource.h"

namespace Chewy {

Resource::Resource(Common::String filename) {
	_stream.open(filename);
	uint32 magicBytes = MKTAG('N', 'G', 'S', '\0');
	if (_stream.readUint32BE() != magicBytes)
		error("Invalid resource - %s", filename.c_str());
	_resType = (ResourceType)_stream.readUint16LE();
	_chunkCount = _stream.readUint16LE();

	for (uint i = 0; i < _chunkCount; i++) {
		Chunk cur;
		cur.size = _stream.readUint32LE();
		cur.type = (ResourceType)_stream.readUint16LE();
		cur.pos = _stream.pos();

		if (cur.type == kResourceTBF) {
			cur.pos += TBF_CHUNK_HEADER_SIZE;
			cur.size -= TBF_CHUNK_HEADER_SIZE;
			readTBFChunk();
		}

		_stream.skip(cur.size);
		_chunkList.push_back(cur);
	}
}

Resource::~Resource() {
	_chunkList.clear();
	_tbfChunkList.clear();
	_stream.close();
}

void Resource::readTBFChunk() {
	TBFChunk cur;
	if (_stream.readUint32BE() != MKTAG('T', 'B', 'F', '\0'))
		error("Corrupt TBF resource");

	cur.screenMode = _stream.readUint16LE();
	cur.compressionFlag = _stream.readUint16LE();
	cur.unpackedSize = _stream.readUint32LE();
	cur.width = _stream.readUint16LE();
	cur.height = _stream.readUint16LE();
	for (int j = 0; j < 3 * 256; j++)
		cur.palette[j] = _stream.readByte() << 2;

	_tbfChunkList.push_back(cur);
}

uint32 Resource::getChunkCount() const {
	return _chunkList.size();
}

Chunk *Resource::getChunk(int num) {
	return &_chunkList[num];
}

TBFChunk *Resource::getTBFChunk(int num) {
	assert(_resType == kResourceTGP);
	return &_tbfChunkList[num];
}

byte *Resource::getChunkData(int num) {
	Chunk *chunk = &_chunkList[num];
	byte *data;

	_stream.seek(chunk->pos, SEEK_SET);

	if (chunk->type == kResourceTBF) {
		TBFChunk *tbfChunk = &_tbfChunkList[num];
		data = new byte[tbfChunk->unpackedSize];

		if (!tbfChunk->compressionFlag) {
			_stream.read(data, chunk->size);
		} else {
			// Compressed images are packed using a very simple RLE compression
			byte count;
			byte value;
			uint32 outPos = 0;

			for (uint i = 0; i < (chunk->size) / 2 && outPos < tbfChunk->unpackedSize; i++) {
				count = _stream.readByte();
				value = _stream.readByte();
				for (byte j = 0; j < count; j++) {
					data[outPos++] = value;
				}
			}
		}
	} else if (chunk->type == kResourceVOC) {
		// Voice files are split in blocks, so reassemble them here
		byte blocksRemaining;
		uint32 totalLength = 0;
		uint32 blockSize;
		
		// Find the total length of the voice file
		do {
			blocksRemaining = _stream.readByte();
			blockSize =
				_stream.readByte() +
				(_stream.readByte() << 8) +
				(_stream.readByte() << 16);

			totalLength += blockSize;
			_stream.skip(blockSize);
		} while (blocksRemaining > 1);

		// Read the voice data
		data = new byte[totalLength];
		byte *ptr = data;

		_stream.seek(chunk->pos, SEEK_SET);

		do {
			blocksRemaining = _stream.readByte();
			blockSize =
				 _stream.readByte() +
				(_stream.readByte() << 8) +
				(_stream.readByte() << 16);
			
			_stream.read(ptr, blockSize);
			ptr += blockSize;
		} while (blocksRemaining > 1);
	} else {
		data = new byte[chunk->size];
		_stream.read(data, chunk->size);
	}

	return data;
}

} // End of namespace Chewy
