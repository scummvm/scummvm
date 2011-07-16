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
 */

#ifndef NEVERHOOD_RESOURCE_H
#define NEVERHOOD_RESOURCE_H

#include "common/str.h"
#include "neverhood/neverhood.h"
#include "neverhood/graphics.h"
#include "neverhood/staticdata.h"

namespace Neverhood {

class SpriteResource {
public:
	SpriteResource(NeverhoodEngine *vm);
	~SpriteResource();
	void draw(byte *dest, int destPitch, bool flipX, bool flipY);
	bool load(uint32 fileHash);
	bool load2(uint32 fileHash);
	void unload();
	const NDimensions& getDimensions() { return _dimensions; }
	NPoint& getPosition() { return _position; }
	bool isRle() const { return _rle; }
	byte *getPixels() const { return _pixels; }
protected:
	NeverhoodEngine *_vm;
	int _resourceHandle;
	NDimensions _dimensions;
	NPoint _position;
	byte *_pixels;
	bool _rle;
};

class PaletteResource {
public:
	PaletteResource(NeverhoodEngine *vm);
	~PaletteResource();
	bool load(uint32 fileHash);
	void unload();
	void copyPalette(byte *destPalette);
	byte *palette() { return _palette; }
protected:
	NeverhoodEngine *_vm;
	int _resourceHandle;
	byte *_palette;
};

struct AnimFrameInfo {
	uint32 frameHash;
	int16 counter;
	NDrawRect rect;
	int16 deltaX, deltaY;
	NDrawRect deltaRect;
	uint16 field_1A; 
	uint32 spriteDataOffs;
};

class AnimResource {
public:
	AnimResource(NeverhoodEngine *vm);
	~AnimResource();
	void draw(uint frameIndex, byte *dest, int destPitch, bool flipX, bool flipY);
	bool load(uint32 fileHash);
	void unload();
	void clear();
	void clear2();
	bool loadInternal(uint32 fileHash);
	void unloadInternal();
	uint getFrameCount() const { return _frames.size(); }
	const AnimFrameInfo& getFrameInfo(int16 index) const { return _frames[index]; }
	int16 getFrameIndex(uint32 frameHash);
	void setReplEnabled(bool value) { _replEnabled = value; }
	void setRepl(byte oldColor, byte newColor);
protected:
	NeverhoodEngine *_vm;
	int _resourceHandle;
	int16 _width, _height;
	byte *_currSpriteData;
	uint32 _fileHash;
	byte *_paletteData;
	byte *_spriteData;
	bool _replEnabled;
	byte _replOldColor;
	byte _replNewColor;
	Common::Array<AnimFrameInfo> _frames;
};

class MouseCursorResource {
public:
	MouseCursorResource(NeverhoodEngine *vm);
	void load(uint32 fileHash);
	void unload();
	NDrawRect& getRect();
	void draw(int frameNum, byte *dest, int destPitch);
	int getCursorNum() const { return _cursorNum; }
	void setCursorNum(int value) { _cursorNum = value; }
protected:
	int _cursorNum;
	SpriteResource _cursorSprite;
	NDrawRect _rect;
	uint32 _currFileHash;
};

class TextResource {
public:
	TextResource(NeverhoodEngine *vm);
	~TextResource();
	void load(uint32 fileHash);
	void unload();
	const char *getString(uint index, const char *&textEnd);
	uint getCount() const { return _count;}
protected:
	NeverhoodEngine *_vm;
	int _resourceHandle;
	byte *_textData;
	uint _count;
};

/* DataResource
	1	Single NPoint
	2	Array of NPoints
	3	Array of NRects
	4	MessageList
	5	SubRectList
	6	RectList
*/

class DataResource {
public:
	DataResource(NeverhoodEngine *vm);
	~DataResource();
	void load(uint32 fileHash);
	void unload();
	NPoint getPoint(uint32 nameHash);
	NPointArray *getPointArray(uint32 nameHash);
	HitRectList *getHitRectList();
	MessageList *getMessageListAtPos(int16 klaymanX, int16 klaymanY, int16 mouseX, int16 mouseY);
protected:

	struct DRDirectoryItem {
		uint32 nameHash;
		uint16 offset; 
		uint16 type;
	};

	struct DRRect {
		NRect rect;
		uint16 subRectIndex;
	};

	struct DRSubRect {
		NRect rect;
		uint32 messageListHash;
		uint16 messageListItemIndex;
	};

	typedef Common::Array<DRSubRect> DRSubRectList;

	NeverhoodEngine *_vm;
	int _resourceHandle;
	Common::Array<DRDirectoryItem> _directory;
	Common::Array<NPoint> _points;
	Common::Array<NPointArray*> _pointArrays;
	Common::Array<NRectArray*> _rectArrays;
	Common::Array<HitRectList*> _hitRectLists;
	Common::Array<MessageList*> _messageLists;
	Common::Array<DRRect> _drRects;
	Common::Array<DRSubRectList*> _drSubRectLists;
	DataResource::DRDirectoryItem *findDRDirectoryItem(uint32 nameHash, uint16 type); 
};

// TODO: Dummy class atm

class SoundResource {
public:
	SoundResource(NeverhoodEngine *vm);
	bool isPlaying();
	void load(uint32 fileHash);
	void play(uint32 fileHash, bool looping = false);
	void play();
protected:
	NeverhoodEngine *_vm;	
};

uint32 calcHash(const char *value);

} // End of namespace Neverhood

#endif /* NEVERHOOD_RESOURCE_H */
