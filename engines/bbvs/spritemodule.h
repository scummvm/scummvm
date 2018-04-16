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

#ifndef BBVS_SPRITEMODULE_H
#define BBVS_SPRITEMODULE_H

#include "common/array.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/rect.h"
#include "common/str.h"

namespace Bbvs {

struct Sprite {
	int type;
	int xOffs, yOffs;
	int width, height;
	byte *data;
	uint32 offset;
	byte *getRow(int y);
};

struct Palette {
	byte *data;
	int start, count;
};

class SpriteModule {
public:
	SpriteModule();
	~SpriteModule();
	void load(const char *filename);
	int getSpriteCount() { return _spritesCount; }
	Sprite getSprite(int index);
	Palette getPalette();
protected:
	byte *_spriteData;
	int _spriteDataSize;
	int _spritesCount;
	uint32 _spriteTblOffs;
	uint32 _paletteOffs;
	int _paletteStart, _paletteCount;
	void unload();
};

} // End of namespace Bbvs

#endif // BBVS_SPRITEMODULE_H
