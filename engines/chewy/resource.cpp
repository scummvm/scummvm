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

#include "common/debug.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "chewy/chewy.h"
#include "chewy/resource.h"

namespace Chewy {

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

uint8 *Resource::getChunkData(uint num) {
	assert(num < _chunkList.size());

	Chunk *chunk = &_chunkList[num];
	uint8 *data = new uint8[chunk->size];

	_stream.seek(chunk->pos, SEEK_SET);
	_stream.read(data, chunk->size);
	if (_encrypted)
		decrypt(data, chunk->size);

	return data;
}

void Resource::initSprite(Common::String filename) {
	uint32 nextSpriteOffset;

	_resType = kResourceTAF;
	_encrypted = false;
	/*screenMode = */_stream.readUint16LE();
	_chunkCount = _stream.readUint16LE();
	_stream.skip(4);
	_stream.skip(3 * 256);
	nextSpriteOffset = _stream.readUint32LE();
	_stream.skip(2 + 1);
	if ((int32)nextSpriteOffset != _stream.pos())
		error("Invalid sprite resource - %s", filename.c_str());

	for (uint i = 0; i < _chunkCount; i++) {
		Chunk cur;

		cur.pos = _stream.pos();
		cur.type = kResourceTAF;

		_stream.skip(2 + 2 + 2);
		nextSpriteOffset = _stream.readUint32LE();
		uint32 spriteImageOffset = _stream.readUint32LE();
		_stream.skip(1);

		if ((int32)spriteImageOffset != _stream.pos())
			error("Invalid sprite resource - %s", filename.c_str());

		cur.size = nextSpriteOffset - cur.pos - 15;

		_stream.skip(cur.size);
		_chunkList.push_back(cur);
	}
}

void Resource::unpackRLE(uint8 *buffer, uint32 compressedSize, uint32 uncompressedSize) {

	uint8 count;
	uint8 value;
	uint32 outPos = 0;

	for (uint i = 0; i < (compressedSize) / 2 && outPos < uncompressedSize; i++) {
		count = _stream.readByte();
		value = _stream.readByte();
		for (uint8 j = 0; j < count; j++) {
			buffer[outPos++] = value;
		}
	}
}

void Resource::decrypt(uint8 *data, uint32 size) {
	uint8 *c = data;

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
	_stream.skip(4 + 4 + 1);

	taf->data = new uint8[taf->width * taf->height];

	if (!taf->compressionFlag)
		_stream.read(taf->data, chunk->size);
	else
		unpackRLE(taf->data, chunk->size, taf->width * taf->height);

	return taf;
}

TBFChunk *BackgroundResource::getImage(uint num, bool fixPalette) {
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
		tbf->palette[j] = fixPalette ? (_stream.readByte() << 2) & 0xff : _stream.readByte();

	tbf->data = new uint8[tbf->size];

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

	uint8 blocksRemaining;
	uint32 totalLength = 0;
	uint32 blockSize;

	do {
		blocksRemaining = _stream.readByte();

		uint8 b1 = _stream.readByte();
		uint8 b2 = _stream.readByte();
		uint8 b3 = _stream.readByte();
		blockSize = b1 + (b2 << 8) + (b3 << 16);

		totalLength += blockSize;
		_stream.skip(blockSize);
	} while (blocksRemaining > 1);

	sound->size = totalLength;
	sound->data = new uint8[totalLength];
	uint8 *ptr = sound->data;

	_stream.seek(chunk->pos, SEEK_SET);

	do {
		blocksRemaining = _stream.readByte();

		uint8 b1 = _stream.readByte();
		uint8 b2 = _stream.readByte();
		uint8 b3 = _stream.readByte();
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

	vid->size = _stream.readUint32LE();
	vid->frameCount = _stream.readUint16LE();
	vid->width = _stream.readUint16LE();
	vid->height = _stream.readUint16LE();
	vid->frameDelay = _stream.readUint32LE();
	vid->firstFrameOffset = _stream.readUint32LE();

	return vid;
}

Common::SeekableReadStream *VideoResource::getVideoStream(uint num) {
	assert(num < _chunkList.size());

	Chunk *chunk = &_chunkList[num];
	return new Common::SeekableSubReadStream(&_stream, chunk->pos, chunk->pos + chunk->size);
}

}
