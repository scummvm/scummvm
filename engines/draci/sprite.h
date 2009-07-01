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

#ifndef DRACI_SPRITE_H
#define DRACI_SPRITE_H

#include "draci/surface.h"
#include "draci/font.h"

namespace Draci {

class Drawable {

friend class Sprite;
friend class Text;

public:
	virtual void draw(Surface *surface) const = 0;
	virtual ~Drawable() {};
	
	virtual uint16 getWidth() { return _width; }
	virtual uint16 getHeight() { return _height; }

	virtual uint16 getX() { return _x; }
	virtual uint16 getY() { return _y; }
	virtual uint16 getZ() { return _z; }

	virtual void setX(uint16 x) { _x = x; }
	virtual void setY(uint16 y) { _y = y; }
	virtual void setZ(uint16 z) { _z = z; }
	
private:
	uint16 _width;	//!< Width of the sprite
	uint16 _height;	//!< Height of the sprite
	uint16 _x, _y;	//!< Sprite coordinates
	uint16 _z; 		//!< Sprite depth position
};

/**
 *  Represents a Draci Historie sprite. Supplies two constructors; one for
 *  loading a sprite from a raw data buffer and one for loading a sprite in
 *  the Draci sprite format. Supports loading the sprite from a column-wise
 *  format (transforming them to row-wise) since that is the way the sprites 
 *  are stored in the original game files.
 *  
 *  Sprite format:	
 *	[uint16LE] sprite width
 * 	[uint16LE] sprite height
 *	[height * width bytes] image pixels stored column-wise, one byte per pixel
 */

class Sprite : public Drawable {

public:
	Sprite(byte *raw_data, uint16 width, uint16 height, uint16 x = 0, uint16 y = 0, 
		bool columnwise = false); 
	
	Sprite(byte *sprite_data, uint16 length, uint16 x = 0, uint16 y = 0, 
		bool columnwise = false); 

	~Sprite();

	void draw(Surface *surface) const;

	const byte *getBuffer() const { return _data; }

private:
	byte *_data;	//!< Pointer to a buffer containing raw sprite data (row-wise)
};

class Text : public Drawable {
	
public:
	Text(const Common::String &str, Font *font, byte fontColour, 
		uint16 x = 0, uint16 y = 0, uint spacing = 0);
	~Text();
	
	void setText(const Common::String &str);
	void setColour(byte fontColour);
	
	void draw(Surface *surface) const;

private:
	byte *_text;
	uint _length;
	uint8 _colour;
	uint _spacing;
	Font *_font;
};
	
} // End of namespace Draci

#endif // DRACI_SPRITE_H
