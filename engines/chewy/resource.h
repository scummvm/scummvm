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
#include "common/memstream.h"
#include "common/random.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace Chewy {

enum ResourceType {
	kResourceUnknown = -1,
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
	uint8 palette[3 * 256];
	uint8 *data;
};

// TAF (sprite) image data chunk header - 15 bytes
struct TAFChunk {
	uint16 compressionFlag;
	uint16 width;
	uint16 height;
	// 4 bytes next sprite offset
	// 4 bytes sprite image offset
	// 1 byte padding
	uint8 *data;
};

// Sound chunk header
struct SoundChunk {
	uint32 size;
	uint8 *data;
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

// Dialog chunk header (AdsBlock)
// Original values are in diah.adh, and are synced
// to saved games
struct DialogChunk {
	bool show[6];
	uint8 next[6];
	uint8 flags[6];
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
	uint32 getSize() const {
		return _stream.size();
	}
	uint32 findLargestChunk(uint start, uint end);
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

	// Sprite specific
	uint8 _spritePalette[3 * 256];
	uint32 _allSize;
	uint16 _spriteCorrectionsCount;
	uint16 *_spriteCorrectionsTable;

	ChunkList _chunkList;
};

class SpriteResource : public Resource {
public:
	SpriteResource(Common::String filename) : Resource(filename) {}
	virtual ~SpriteResource() {}

	TAFChunk *getSprite(uint num);
	uint32 getSpriteData(uint num, uint8 **buf, bool initBuffer);
	uint8 *getSpritePalette() { return _spritePalette; }
	uint32 getAllSize() { return _allSize; }
	uint16 getSpriteCorrectionsCount() { return _spriteCorrectionsCount; }
	uint16 *getSpriteCorrectionsTable() { return _spriteCorrectionsTable; }
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

class DialogResource : public Resource {
public:
	DialogResource(Common::String filename);
	virtual ~DialogResource();

	DialogChunk *getDialog(uint dialog, uint block);
	bool isItemShown(uint dialog, uint block, uint num);
	void setItemShown(uint dialog, uint block, uint num, bool shown);
	bool hasExitBit(uint dialog, uint block, uint num);
	bool hasRestartBit(uint dialog, uint block, uint num);
	bool hasShowBit(uint dialog, uint block, uint num);
	uint8 getNextBlock(uint dialog, uint block, uint num);

	void loadStream(Common::SeekableReadStream *s);
	void saveStream(Common::WriteStream *s);

	uint32 getStreamSize() const {
		return _stream.size();
	}

private:
	Common::MemorySeekableReadWriteStream *_dialogStream;
	byte *_dialogBuffer;
};

} // namespace Chewy

#endif
