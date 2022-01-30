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

#ifndef CHEWY_RESOURCE_H
#define CHEWY_RESOURCE_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/util.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace Chewy {

enum ResourceType {
	kResourcePCX = 0,
	kResourceTBF = 1,
	kResourceTAF = 2,
	kResourceTFF = 3,
	kResourceVOC = 4,
	kResourceTPF = 5,
	kResourceTMF = 6,
	kResourceMOD = 7,
	kResourceRAW = 8,
	kResourceLBM = 9,
	kResourceRDI = 10,
	kResourceTXT = 11,
	kResourceIIB = 12,
	kResourceSIB = 13,
	kResourceEIB = 14,
	kResourceATS = 15,
	kResourceSAA = 16,
	kResourceFLC = 17,
	kResourceAAD = 18,
	kResourceADS = 19,
	kResourceADH = 20,
	kResourceTGP = 21,
	kResourceTVP = 22,
	kResourceTTP = 23,
	kResourceTAP = 24,
	kResourceCFO = 25,
	kResourceTCF = 26
};


struct Chunk {
	uint32 size;
	uint16 num;
	ResourceType type;
	uint32 pos;
};


struct TBFChunk {


	uint16 screenMode;
	uint16 compressionFlag;
	uint32 size;
	uint16 width;
	uint16 height;
	uint8 palette[3 * 256];
	uint8 *data;
};


struct TAFChunk {
	uint16 compressionFlag;
	uint16 width;
	uint16 height;



	uint8 *data;
};


struct SoundChunk {
	uint32 size;
	uint8 *data;
};


struct VideoChunk {

	uint32 size;
	uint16 frameCount;
	uint16 width;
	uint16 height;
	uint32 frameDelay;
	uint32 firstFrameOffset;
};

enum VideoFrameType {
	kVideoFrameNormal = 0xF1FA,
	kVideoFrameCustom = 0xFAF1
};

typedef Common::Array<Chunk> ChunkList;
typedef Common::Array<TBFChunk> TBFChunkList;

class Resource {
public:
	Resource(Common::String filename);
	virtual ~Resource();

	ResourceType getType() const {
		return _resType;
	}
	uint32 getChunkCount() const;
	Chunk *getChunk(uint num);
	virtual uint8 *getChunkData(uint num);

protected:
	void initSprite(Common::String filename);
	void unpackRLE(uint8 *buffer, uint32 compressedSize, uint32 uncompressedSize);
	void decrypt(uint8 *data, uint32 size);

	Common::File _stream;
	uint16 _chunkCount;
	ResourceType _resType;
	bool _encrypted;

	ChunkList _chunkList;
};

class SpriteResource : public Resource {
public:
	SpriteResource(Common::String filename) : Resource(filename) {}
	virtual ~SpriteResource() {}

	TAFChunk *getSprite(uint num);
};

class BackgroundResource : public Resource {
public:
	BackgroundResource(Common::String filename) : Resource(filename) {}
	virtual ~BackgroundResource() {}

	TBFChunk *getImage(uint num, bool fixPalette);
};

class SoundResource : public Resource {
public:
	SoundResource(Common::String filename) : Resource(filename) {}
	virtual ~SoundResource() {}

	SoundChunk *getSound(uint num);
};

class VideoResource : public Resource {
public:
	VideoResource(Common::String filename) : Resource(filename) {}
	virtual ~VideoResource() {}

	VideoChunk *getVideoHeader(uint num);
	Common::SeekableReadStream *getVideoStream(uint num);
};

}

#endif
