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

#ifndef MADS_ASSETS_H
#define MADS_ASSETS_H

#include "common/scummsys.h"
#include "common/array.h"
#include "mads/palette.h"

namespace MADS {

enum AssetFlags {
	ASSET_TRANSLATE = 1, ASSET_HEADER_ONLY = 2, ASSET_CHAR_INFO = 4,
	ASSET_SPINNING_OBJECT = 8
};

class MADSEngine;
class MSprite;
class MSurface;

struct SpriteAssetFrame {
	uint32 _stream;
	Common::Rect _bounds;
	uint32 _comp;
	MSprite *_frame;
};

class SpriteSetCharInfo {
public:
	SpriteSetCharInfo(Common::SeekableReadStream *s);

	int _totalFrames;
	int _numEntries;
	int _startFrames[16];
	int _stopFrames[16];
	int _ticksList[16];
	int _velocity;
	int _ticksAmount;
	int _centerOfGravity;
};

class SpriteAsset {
private:
	MADSEngine *_vm;
	byte _palette[PALETTE_SIZE];
	int _colorCount;
	uint32 _srcSize;
	int _frameRate, _pixelSpeed;
	int _maxWidth, _maxHeight;
	int _frameCount;
	Common::Array<uint32> _frameOffsets;
	Common::Array<SpriteAssetFrame> _frames;
	uint8 _mode;
	bool _isBackground;

	/**
	 * Load the data for the asset
	 */
	void load(Common::SeekableReadStream *stream, int flags);
public:
	SpriteSetCharInfo *_charInfo;
	int _usageIndex;
public:
	/**
	 * Constructor
	 */
	SpriteAsset(MADSEngine *vm, const Common::String &resourceName, int flags);

	/**
	 * Constructor
	 */
	SpriteAsset(MADSEngine *vm, Common::SeekableReadStream *stream, int flags);

	/**
	 * Destructor
	 */
	~SpriteAsset();

	int getCount() { return _frameCount; }
	int getFrameRate() const { return _frameRate; }
	int getPixelSpeed() const { return _pixelSpeed; }
	Common::Point getFramePos(int index) { assert (index < _frameCount); return Common::Point(_frames[index]._bounds.left, _frames[index]._bounds.top); }
	int getFrameWidth(int index) { assert (index < _frameCount); return _frames[index]._bounds.width(); }
	int getFrameHeight(int index) { assert (index < _frameCount); return _frames[index]._bounds.height(); }
	int getMaxFrameWidth() const { return _maxWidth; }
	int getMaxFrameHeight() const { return _maxHeight; }
	MSprite *getFrame(int frameIndex);
	byte *getPalette() { return _palette; }
	int getColorCount() { return _colorCount; }
	bool isBackground() const { return _isBackground; }
};

} // End of namespace MADS

#endif /* MADS_ASSETS_H */
