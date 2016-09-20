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
	const uint32 headerGeneric = MKTAG('N', 'G', 'S', '\0');
	const uint32 headerTxtDec  = MKTAG('T', 'C', 'F', '\0');
	const uint32 headerTxtEnc  = MKTAG('T', 'C', 'F', '\1');

	_stream.open(filename);

	uint32 header = _stream.readUint32BE();
	bool isText = header == headerTxtDec || header == headerTxtEnc;

	if (header != headerGeneric && !isText)
		error("Invalid resource - %s", filename.c_str());

	if (isText) {
		_resType = kResourceTCF;
		_encrypted = (header == headerTxtEnc);
	} else {
		_resType = (ResourceType)_stream.readUint16LE();
		_encrypted = false;
	}

	_chunkCount = _stream.readUint16LE();

	for (uint i = 0; i < _chunkCount; i++) {
		Chunk cur;
		cur.size = _stream.readUint32LE();

		if (!isText)
			cur.type = (ResourceType)_stream.readUint16LE();
		else
			cur.num = _stream.readUint16LE();

		cur.pos = _stream.pos();

		_stream.skip(cur.size);
		_chunkList.push_back(cur);
	}
}

Resource::~Resource() {
	_chunkList.clear();
	_stream.close();
}

uint32 Resource::getChunkCount() const {
	return _chunkList.size();
}

Chunk *Resource::getChunk(uint num) {
	assert(num < _chunkList.size());

	return &_chunkList[num];
}

byte *Resource::getChunkData(uint num) {
	assert(num < _chunkList.size());

	Chunk *chunk = &_chunkList[num];
	byte *data = new byte[chunk->size];

	_stream.seek(chunk->pos, SEEK_SET);
	_stream.read(data, chunk->size);

	return data;
}

TBFChunk *BackgroundResource::getImage(uint num) {
	assert(num < _chunkList.size());

	Chunk *chunk = &_chunkList[num];
	TBFChunk *tbf = new TBFChunk();

	_stream.seek(chunk->pos, SEEK_SET);

	if (_stream.readUint32BE() != MKTAG('T', 'B', 'F', '\0'))
		error("Corrupt TBF resource");

	tbf->screenMode = _stream.readUint16LE();
	tbf->compressionFlag = _stream.readUint16LE();
	tbf->size = _stream.readUint32LE();
	tbf->width = _stream.readUint16LE();
	tbf->height = _stream.readUint16LE();
	for (int j = 0; j < 3 * 256; j++)
		tbf->palette[j] = _stream.readByte() << 2;

	tbf->data = new byte[tbf->size];

	if (!tbf->compressionFlag) {
		_stream.read(tbf->data, chunk->size);
	}
	else {
		// Compressed images are packed using a very simple RLE compression
		byte count;
		byte value;
		uint32 outPos = 0;

		for (uint i = 0; i < (chunk->size) / 2 && outPos < tbf->size; i++) {
			count = _stream.readByte();
			value = _stream.readByte();
			for (byte j = 0; j < count; j++) {
				tbf->data[outPos++] = value;
			}
		}
	}

	return tbf;
}

SoundChunk *SoundResource::getSound(uint num) {
	assert(num < _chunkList.size());

	Chunk *chunk = &_chunkList[num];
	SoundChunk *sound = new SoundChunk();

	_stream.seek(chunk->pos, SEEK_SET);

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
	sound->size = totalLength;
	sound->data = new byte[totalLength];
	byte *ptr = sound->data;

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

	return sound;
}

Common::String TextResource::getText(uint num) {
	assert(num < _chunkList.size());

	Chunk *chunk = &_chunkList[num];
	Common::String str;
	byte *data = new byte[chunk->size];

	_stream.seek(chunk->pos, SEEK_SET);

	_stream.read(data, chunk->size);

	if (_encrypted) {
		byte *c = data;

		for (uint i = 0; i < chunk->size; i++) {
			*c = -(*c);
			++c;
		}
	}

	str = (char *)data;
	delete[] data;

	return str;
}

} // End of namespace Chewy
