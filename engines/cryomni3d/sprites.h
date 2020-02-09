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
	void setupMapTable(const uint *table, uint size);

	void setSpriteHotspot(uint spriteId, uint x, uint y);

	void replaceSprite(uint oldSpriteId, uint newSpriteId);

	uint getSpritesCount() const;

	void replaceSpriteColor(uint spriteId, byte currentColor, byte newColor);

	const Graphics::Surface &getSurface(uint spriteId) const;
	const Graphics::Cursor &getCursor(uint spriteId) const;

	uint revMapSpriteId(uint id) const;
	uint calculateSpriteId(uint baseId, uint offset) const;

	byte getKeyColor(uint spriteId) const { return 0; }

private:
	class CryoCursor : public Graphics::Cursor {
	public:
		uint16 getWidth() const override { return _width; }
		uint16 getHeight() const override { return _height; }
		uint16 getHotspotX() const override { return _offX; }
		uint16 getHotspotY() const override { return _offY; }
		byte getKeyColor() const override { return 0; }

		const byte *getSurface() const override { return _data; }

		const byte *getPalette() const override { return nullptr; }
		byte getPaletteStartIndex() const override { return 0; }
		uint16 getPaletteCount() const override { return 0; }

		uint setup(uint16 width, uint16 height);

		uint16 _width;
		uint16 _height;
		int16 _offX;
		int16 _offY;
		uint _constantId;

		byte *_data;

		uint refCnt;

		CryoCursor();
		~CryoCursor() override;
	};

	// Pointer to avoid to mutate Sprites when asking for a cursor
	Graphics::Surface *_surface;
	Common::Array<CryoCursor *> _cursors;
	Common::Array<uint> *_map;
};

} // End of namespace CryOmni3D

#endif
