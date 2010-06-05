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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
#ifndef M4_ASSETS_H
#define M4_ASSETS_H

#include "common/scummsys.h"
#include "common/stream.h"

#include "m4/sprite.h"

namespace M4 {

// Sequence chunks
#define CHUNK_SCEN MKID_BE('SCEN')
#define CHUNK_MACH MKID_BE('MACH')
#define CHUNK_SEQU MKID_BE('SEQU')
#define CHUNK_DATA MKID_BE('DATA')
#define CHUNK_CELS MKID_BE('CELS')

// Sprite chunks
#define HEAD_M4SS MKID_BE('M4SS')	//'M4SS'
#define CELS__PAL MKID_BE(' PAL')	//' PAL'
#define CELS___SS MKID_BE('  SS')	//'  SS'

class MadsM4Engine;
class Palette;

class BaseAsset {
public:
	BaseAsset(MadsM4Engine *vm);
	~BaseAsset();
	const Common::String getName() const { return _name; }
protected:
	MadsM4Engine *_vm;
	Common::String _name;
};

class MachineAsset : public BaseAsset {
public:
	MachineAsset(MadsM4Engine *vm, Common::SeekableReadStream* stream, int size, const char *name);
	~MachineAsset();
	void getCode(byte *&code, uint32 &codeSize);
	uint32 getStateOffset(uint32 state);
protected:
	Common::Array<uint32> _stateTable;
	byte *_code;
	uint32 _codeSize;
};

class SequenceAsset : public BaseAsset {
public:
	SequenceAsset(MadsM4Engine *vm, Common::SeekableReadStream* stream, int size, const char *name);
	~SequenceAsset();
	void getCode(byte *&code, uint32 &codeSize);
	int localVarCount() const { return _localVarCount; }
protected:
	int _localVarCount;
	byte *_code;
	uint32 _codeSize;
};

class DataAsset : public BaseAsset {
public:
	DataAsset(MadsM4Engine *vm, Common::SeekableReadStream* stream, int size, const char *name);
	~DataAsset();
	int getCount() const { return _recCount; }
	long *getRow(int index);
protected:
	long *_data;
	uint32 _recSize, _dataSize;
	int _recCount;
};

struct SpriteAssetFrame {
	uint32 stream;
	int x, y, w, h;
	uint32 comp;
	M4Sprite *frame;
};

class SpriteAsset : public BaseAsset {
public:
	SpriteAsset(MadsM4Engine *vm, Common::SeekableReadStream* stream, int size, const char *name, bool asStream = false);
	SpriteAsset(MadsM4Engine *vm, const char *name);
	~SpriteAsset();
	void loadM4SpriteAsset(MadsM4Engine *vm, Common::SeekableReadStream* stream, bool asStream);
	void loadMadsSpriteAsset(MadsM4Engine *vm, Common::SeekableReadStream* stream);
	int32 getCount() { return _frameCount; }
	int32 getFrameRate() const { return _frameRate; }
	int32 getPixelSpeed() const { return _pixelSpeed; }
	int32 getFrameWidth(int index);
	int32 getFrameHeight(int index);
	int32 getMaxFrameWidth() const { return _maxWidth; }
	int32 getMaxFrameHeight() const { return _maxHeight; }
	bool isBackground() const { return _isBackground; }
	M4Sprite *getFrame(int frameIndex);
	void loadStreamingFrame(M4Sprite *frame, int frameIndex, int destX, int destY);
	RGB8* getPalette() { return _palette; }
	int getColorCount() { return _colorCount; }
	RGBList *getRgbList();
	void translate(RGBList *list, bool isTransparent = false);
	void translate(Palette *palette);
	int32 getFrameSize(int index);
	M4Sprite *operator[](int index) { return getFrame(index); }
protected:
	Common::SeekableReadStream *_stream;
	RGB8 _palette[256];
	uint32 _colorCount;
	uint32 _srcSize;
	int32 _frameRate, _pixelSpeed;
	int _maxWidth, _maxHeight;
	int _frameCount;
	Common::Array<uint32> _frameOffsets;
	Common::Array<SpriteAssetFrame> _frames;
	uint32 _frameStartOffset;
	
	// MADS sprite set fields
	uint8 _mode;
	bool _isBackground;

	int32 parseSprite(bool isBigEndian = false);
	void loadFrameHeader(SpriteAssetFrame &frameHeader, bool isBigEndian = false);
private:
	RGBList *_paletteData;
	Palette *_palInterface;
};

enum AssetType {
	kAssetTypeMACH,
	kAssetTypeSEQU,
	kAssetTypeDATA,
	kAssetTypeCELS
};

enum CallbackHandlers {
	kCallbackTriggerDispatch
};

class AssetManager {
public:

	AssetManager(MadsM4Engine *vm);
	~AssetManager();

	bool clearAssets(AssetType assetType, int32 minHash, int32 maxHash);
	bool loadAsset(const char *assetName, RGB8 *palette);
	int32 addSpriteAsset(const char *assetName, int32 hash, RGB8 *palette);

	// TODO: Move to Palette class
	void restorePalette(RGB8 *palette, byte *data);

	MachineAsset *getMachine(int32 hash);
	SequenceAsset *getSequence(int32 hash);
	DataAsset *getData(int32 hash);
	SpriteAsset *getSprite(int32 hash);
	M4Sprite *getSpriteFrame(int32 hash, int frameIndex);
	int32 getSpriteFrameCount(int32 hash);

protected:
	// TODO: Check if we need _vm
	MadsM4Engine *_vm;

	MachineAsset *_MACH[256];
	SequenceAsset *_SEQU[256];
	DataAsset *_DATA[256];
	SpriteAsset *_CELS[256];

	void convertAssetToLE(byte *assetData, uint32 assetSize);

};

} // End of namespace M4

#endif
