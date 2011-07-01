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

#include "neverhood/neverhood.h"
#include "neverhood/graphics.h"

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
	const NPoint& getPosition() { return _position; }
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
	NRect rect;
	int16 deltaX, deltaY;
	NDrawRect deltaRect;
	uint16 field_1A; 
	uint32 spriteDataOffs;
};

class AnimResource {
public:
	AnimResource(NeverhoodEngine *vm);
	~AnimResource();
	bool load(uint32 fileHash);
	void unload();
	void clear();
	void clear2();
	bool loadInternal(uint32 fileHash);
	void unloadInternal();
	uint getFrameCount() const { return _frames.size(); }
	const AnimFrameInfo& getFrameInfo(int16 index) const { return _frames[index]; }
	int16 getFrameIndex(uint32 frameHash);
protected:
	NeverhoodEngine *_vm;
	int _resourceHandle;
	int16 _width, _height;
	byte *_currSpriteData;
	uint32 _fileHash;
	byte *_paletteData;
	byte *_spriteData;
	bool _replEnabled;
	byte _replOldByte;
	byte _replNewByte;
	Common::Array<AnimFrameInfo> _frames;
};


} // End of namespace Neverhood

#endif /* NEVERHOOD_RESOURCE_H */
