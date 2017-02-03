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
#include "common/substream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "chewy/chewy.h"
#include "chewy/resource.h"

namespace Chewy {

// Resource files - TODO:
// ======================
// back/episode1.gep
// cut/blende.rnd
// misc/exit.eib
// misc/inventar.iib
// misc/inventar.sib
// room/test.rdi
// txt/diah.adh
// txt/inv_st.s and txt/room_st.s (inventory/room control bytes)
// txt/inv_use.idx

Resource::Resource(Common::String filename) {
	const uint32 headerGeneric = MKTAG('N', 'G', 'S', '\0');
	const uint32 headerTxtDec  = MKTAG('T', 'C', 'F', '\0');
	const uint32 headerTxtEnc  = MKTAG('T', 'C', 'F', '\1');
	const uint32 headerSprite  = MKTAG('T', 'A', 'F', '\0');

	filename.toLowercase();
	_stream.open(filename);

	uint32 header = _stream.readUint32BE();
	bool isText = (header == headerTxtDec || header == headerTxtEnc);
	bool isSprite = (header == headerSprite);

	if (header != headerGeneric && !isSprite && !isText)
		error("Invalid resource - %s", filename.c_str());

	if (isText) {
		_resType = kResourceTCF;
		_encrypted = (header == headerTxtEnc);
	} else if (isSprite) {
		initSprite(filename);
		return;
	} else {
		_resType = (ResourceType)_stream.readUint16LE();
		_encrypted = false;
	}

	if (filename == "atds.tap")
		_encrypted = true;

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
	if (_encrypted)
		decrypt(data, chunk->size);

	return data;
}

void Resource::initSprite(Common::String filename) {
	uint32 nextSpriteOffset;

	// TAF (sprite) resources are much different than the rest, so we have a
	// separate initializer for them here

	_resType = kResourceTAF;
	_encrypted = false;
	/*screenMode = */_stream.readUint16LE();
	_chunkCount = _stream.readUint16LE();
	_stream.skip(4);		// total size of all sprites
	_stream.skip(3 * 256);	// palette
	nextSpriteOffset = _stream.readUint32LE();
	_stream.skip(2 + 1);	// correction table, padding
	if ((int32)nextSpriteOffset != _stream.pos())
		error("Invalid sprite resource - %s", filename.c_str());

	for (uint i = 0; i < _chunkCount; i++) {
		Chunk cur;

		cur.pos = _stream.pos();
		cur.type = kResourceTAF;

		_stream.skip(2 + 2 + 2);	// compression flag, width, height
		nextSpriteOffset = _stream.readUint32LE();
		uint32 spriteImageOffset = _stream.readUint32LE();
		_stream.skip(1);	// padding

		if ((int32)spriteImageOffset != _stream.pos())
			error("Invalid sprite resource - %s", filename.c_str());

		cur.size = nextSpriteOffset - cur.pos - 15; // 15 = sizeof(TAFChunk)

		_stream.skip(cur.size);
		_chunkList.push_back(cur);
	}
}

void Resource::unpackRLE(byte *buffer, uint32 compressedSize, uint32 uncompressedSize) {
	// Compressed images are packed using a very simple RLE compression
	byte count;
	byte value;
	uint32 outPos = 0;

	for (uint i = 0; i < (compressedSize) / 2 && outPos < uncompressedSize; i++) {
		count = _stream.readByte();
		value = _stream.readByte();
		for (byte j = 0; j < count; j++) {
			buffer[outPos++] = value;
		}
	}
}

void Resource::decrypt(byte *data, uint32 size) {
	byte *c = data;

	for (uint i = 0; i < size; i++) {
		*c = -(*c);
		++c;
	}
}

TAFChunk *SpriteResource::getSprite(uint num) {
	assert(num < _chunkList.size());

	Chunk *chunk = &_chunkList[num];
	TAFChunk *taf = new TAFChunk();

	_stream.seek(chunk->pos, SEEK_SET);

	taf->compressionFlag = _stream.readUint16LE();
	taf->width = _stream.readUint16LE();
	taf->height = _stream.readUint16LE();
	_stream.skip(4 + 4 + 1);	// nextSpriteOffset, spriteImageOffset, padding

	taf->data = new byte[taf->width * taf->height];

	if (!taf->compressionFlag)
		_stream.read(taf->data, chunk->size);
	else
		unpackRLE(taf->data, chunk->size, taf->width * taf->height);

	return taf;
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
		tbf->palette[j] = (_stream.readByte() << 2) & 0xff;

	tbf->data = new byte[tbf->size];

	if (!tbf->compressionFlag)
		_stream.read(tbf->data, chunk->size);
	else
		unpackRLE(tbf->data, chunk->size, tbf->size);

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

		byte b1 = _stream.readByte();
		byte b2 = _stream.readByte();
		byte b3 = _stream.readByte();
		blockSize = b1 + (b2 << 8) + (b3 << 16);

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

		byte b1 = _stream.readByte();
		byte b2 = _stream.readByte();
		byte b3 = _stream.readByte();
		blockSize = b1 + (b2 << 8) + (b3 << 16);

		_stream.read(ptr, blockSize);
		ptr += blockSize;
	} while (blocksRemaining > 1);

	return sound;
}

VideoChunk *VideoResource::getVideoHeader(uint num) {
	assert(num < _chunkList.size());

	Chunk *chunk = &_chunkList[num];
	VideoChunk *vid = new VideoChunk();

	_stream.seek(chunk->pos, SEEK_SET);

	if (_stream.readUint32BE() != MKTAG('C', 'F', 'O', '\0'))
		error("Corrupt video resource");

	vid->size = _stream.readUint32LE();	// always 0
	vid->frameCount = _stream.readUint16LE();
	vid->width = _stream.readUint16LE();
	vid->height = _stream.readUint16LE();
	vid->frameDelay = _stream.readUint32LE();
	vid->firstFrameOffset = _stream.readUint32LE();	// always 22

	return vid;
}

Common::SeekableReadStream *VideoResource::getVideoStream(uint num) {
	assert(num < _chunkList.size());

	Chunk *chunk = &_chunkList[num];
	return new Common::SeekableSubReadStream(&_stream, chunk->pos, chunk->pos + chunk->size);
}

} // End of namespace Chewy
