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

#ifndef CRYOMNI3D_SPRITES_H
#define CRYOMNI3D_SPRITES_H

#include "common/array.h"
#include "common/str.h"

#include "graphics/cursor.h"

namespace Common {
class ReadStream;
}

namespace Graphics {
struct Surface;
}

namespace CryOmni3D {

class Sprites {
public:
	Sprites();
	virtual ~Sprites();

	void loadSprites(Common::ReadStream &spr_fl);
	void setupMapTable(const unsigned int *table, unsigned int size);

	void setSpriteHotspot(unsigned int spriteId, unsigned int x, unsigned int y);

	void replaceSprite(unsigned int oldSpriteId, unsigned int newSpriteId);

	unsigned int getSpritesCount() const;

	const Graphics::Surface &getSurface(unsigned int spriteId) const;
	const Graphics::Cursor &getCursor(unsigned int spriteId) const;

	unsigned int revMapSpriteId(unsigned int id) const;
	unsigned int calculateSpriteId(unsigned int baseId, unsigned int offset) const;

	byte getKeyColor(unsigned int spriteId) const { return 0; }

private:
	class CryoCursor : public Graphics::Cursor {
	public:
		virtual uint16 getWidth() const override { return _width; }
		virtual uint16 getHeight() const override { return _height; }
		virtual uint16 getHotspotX() const override { return _offX; }
		virtual uint16 getHotspotY() const override { return _offY; }
		virtual byte getKeyColor() const override { return 0; }

		virtual const byte *getSurface() const override { return _data; }

		virtual const byte *getPalette() const override { return nullptr; }
		virtual byte getPaletteStartIndex() const override { return 0; }
		virtual uint16 getPaletteCount() const override { return 0; }

		unsigned int setup(uint16 width, uint16 height);

		uint16 _width;
		uint16 _height;
		int16 _offX;
		int16 _offY;
		unsigned int _constantId;

		byte *_data;

		unsigned int refCnt;

		CryoCursor();
		virtual ~CryoCursor();
	};

	// Pointer to avoid to mutate Sprites when asking for a cursor
	Graphics::Surface *_surface;
	Common::Array<CryoCursor *> _cursors;
	Common::Array<unsigned int> *_map;
};

} // End of namespace CryOmni3D

#endif
