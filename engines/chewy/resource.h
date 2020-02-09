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
#include "common/stream.h"
#include "graphics/surface.h"

namespace Chewy {

enum ResourceType {
	kResourcePCX = 0,		// unused
	kResourceTBF = 1,		// background art, contained in TGPs
	kResourceTAF = 2,
	kResourceTFF = 3,
	kResourceVOC = 4,		// speech and SFX, contained in TVPs
	kResourceTPF = 5,		// unused
	kResourceTMF = 6,		// music, similar to a MOD file, contained in details.tap
	kResourceMOD = 7,		// unused
	kResourceRAW = 8,		// unused
	kResourceLBM = 9,		// unused
	kResourceRDI = 10,
	kResourceTXT = 11,
	kResourceIIB = 12,
	kResourceSIB = 13,
	kResourceEIB = 14,
	kResourceATS = 15,		// unused
	kResourceSAA = 16,		// unused
	kResourceFLC = 17,		// unused
	kResourceAAD = 18,		// unused
	kResourceADS = 19,		// unused
	kResourceADH = 20,		// used in txt/diah.adh
	kResourceTGP = 21,		// container for background art, used in back/comic.tgp, back/episode1.tgp and back/gbook.tgp
	kResourceTVP = 22,		// container for speech, used in sound/speech.tvp
	kResourceTTP = 23,		// unused
	kResourceTAP = 24,		// container for sound effects, music and cutscenes, used in sound/details.tap and cut/cut.tap
	kResourceCFO = 25,		// unused
	kResourceTCF = 26		// error messages, used in err/err_e.tcf (English) and err/err_d.tcf (German)
};

// Generic chunk header
struct Chunk {
	uint32 size;
	uint16 num;	// same as the type below, used in chunks where the type is substituted with count
	ResourceType type;
	uint32 pos;	// position of the actual data
};

// TBF (background) chunk header
struct TBFChunk {
	// TBF chunk header
	// ID (TBF, followed by a zero)
	uint16 screenMode;
	uint16 compressionFlag;
	uint32 size;
	uint16 width;
	uint16 height;
	byte palette[3 * 256];
	byte *data;
};

// TAF (sprite) image data chunk header - 15 bytes
struct TAFChunk {
	uint16 compressionFlag;
	uint16 width;
	uint16 height;
	// 4 bytes next sprite offset
	// 4 bytes sprite image offset
	// 1 byte padding
	byte *data;
};

// Sound chunk header
struct SoundChunk {
	uint32 size;
	byte *data;
};

// Video chunk header
struct VideoChunk {
	// ID (CFA, followed by a zero)
	uint32 size;
	uint16 frameCount;
	uint16 width;
	uint16 height;
	uint32 frameDelay;	// in ms
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

	ResourceType getType() const { return _resType; }
	uint32 getChunkCount() const;
	Chunk *getChunk(uint num);
	virtual byte *getChunkData(uint num);

protected:
	void initSprite(Common::String filename);
	void unpackRLE(byte *buffer, uint32 compressedSize, uint32 uncompressedSize);
	void decrypt(byte *data, uint32 size);

	Common::File _stream;
	uint16 _chunkCount;
	ResourceType _resType;
	bool _encrypted;

	ChunkList _chunkList;
};

class SpriteResource : public Resource {
public:
	SpriteResource(Common::String filename) : Resource(filename) {}
	~SpriteResource() override {}

	TAFChunk *getSprite(uint num);
};

class BackgroundResource : public Resource {
public:
	BackgroundResource(Common::String filename) : Resource(filename) {}
	~BackgroundResource() override {}

	TBFChunk *getImage(uint num);
};

class SoundResource : public Resource {
public:
	SoundResource(Common::String filename) : Resource(filename) {}
	~SoundResource() override {}

	SoundChunk *getSound(uint num);
};

class VideoResource : public Resource {
public:
	VideoResource(Common::String filename) : Resource(filename) {}
	~VideoResource() override {}

	VideoChunk *getVideoHeader(uint num);
	Common::SeekableReadStream *getVideoStream(uint num);
};

} // End of namespace Chewy

#endif
