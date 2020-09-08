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
 * $URL: https://scummvm-startrek.googlecode.com/svn/trunk/graphics.h $
 * $Id: graphics.h 2 2009-09-12 20:13:40Z clone2727 $
 *
 */

#ifndef STARTREK_SPRITE_H
#define STARTREK_SPRITE_H

#include "startrek/bitmap.h"

#include "common/ptr.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "common/stream.h"

using Common::SharedPtr;

namespace StarTrek {


// Note: Rects in Star Trek are considered to have their bottom-right pixel contained in
// the rectangle, but ScummVM rects are not. Functions from Trek have been adapted to use
// ScummVM's rect format. Be wary of off-by-1 errors...

class Sprite : Common::Serializable {
public:
	Common::Point pos;
	uint16 drawPriority;
	uint16 drawPriority2; // If two sprites' drawPriorities are equal, this is checked.
	Common::String field8;
	Bitmap *bitmap;
	uint16 drawMode;
	uint16 textColor;
	bool bitmapChanged;
	bool rect2Valid;
	bool isOnScreen;
	bool field16; // When set, sprite isn't drawn next refresh? (Gets reset to 0 after)
	Common::Rect lastDrawRect; // Rect encompassing the sprite last time it was drawn
	Common::Rect drawRect;     // Rect encompassing the sprite currently
	Common::Rect rectangle2;
	int16 drawX, drawY;

	Sprite();
	virtual ~Sprite();

	void setBitmap(Bitmap *b);
	void setBitmap(Common::MemoryReadStreamEndian *stream);
	void setXYAndPriority(int16 x, int16 y, int16 priority);
	void dontDrawNextFrame();

	/**
	 * Returns a rect containing the sprite's bitmap on the screen.
	 */
	Common::Rect getRect();

	/// NOTE: even after calling this, "bitmap" must be reloaded by the caller.
	void saveLoadWithSerializer(Common::Serializer &ser) override;
};

} // End of namespace StarTrek

#endif
