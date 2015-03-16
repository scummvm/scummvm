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

#ifndef SHERLOCK_SPRITE_H
#define SHERLOCK_SPRITE_H

#include "common/array.h"
#include "common/rect.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace Sherlock {

enum { RLE_ENCODED = 0x0100 };

struct SpriteFrame {
	uint32 _size;
	uint16 _width, _height;
	int _flags;
	Common::Point _offset;
	byte _rleMarker;
	Graphics::Surface _frame;
};

class Sprite: public Common::Array<SpriteFrame> {
private:
	void load(Common::SeekableReadStream &stream, bool skipPal);
	void loadPalette(Common::SeekableReadStream &stream);
	void decompressFrame(SpriteFrame  &frame, const byte *src);
public:
	byte _palette[256 * 3];
public:
    Sprite(Common::SeekableReadStream &stream, bool skipPal = false);
    ~Sprite();
};

} // End of namespace Sherlock

#endif
