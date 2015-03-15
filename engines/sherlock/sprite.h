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

#include "common/stream.h"
#include "common/array.h"
#include "graphics/surface.h"

namespace Sherlock {

struct SpriteFrame {
	byte *data;
	uint32 size;
	uint16 width, height;
	uint16 flags;
	int xofs, yofs;
	byte rleMarker;
	Graphics::Surface *frame;
};

class Sprite {
public:
    Sprite(Common::SeekableReadStream &stream);
    ~Sprite();
	int getFrameCount();
	SpriteFrame *getFrame(int index);
protected:
	Common::Array<SpriteFrame*> _frames;
	void load(Common::SeekableReadStream &stream);
	void decompressFrame(SpriteFrame *frame);
};

} // End of namespace Sherlock

#endif
