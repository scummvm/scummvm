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
	virtual void draw(Surface *surface, bool markDirty = true) const = 0;
	virtual void drawScaled(Surface *surface, bool markDirty = true) const = 0;

	virtual ~Drawable() {};
	
	uint getWidth() const { return _width; }
	uint getHeight() const { return _height; }

	uint getScaledWidth() const { return _scaledWidth; }
	uint getScaledHeight() const { return _scaledHeight; }

	void setScaled(uint width, uint height) { 
		_scaledWidth = width;
		_scaledHeight = height;
	}

	int getX() const { return _x; }
	int getY() const { return _y; }

	void setX(int x) { _x = x; }
	void setY(int y) { _y = y; }

	void setDelay(int delay) { _delay = delay; }
	int getDelay() const { return _delay; }	

	virtual Common::Rect getRect(bool scaled = true) const = 0;
	
private:
	uint _width;		//!< Width of the sprite
	uint _height;		//!< Height of the sprite
	uint _scaledWidth; 	//!< Scaled width of the sprite
	uint _scaledHeight; //!< Scaled height of the sprite
	int _x, _y;			//!< Sprite coordinates

	/** The time a drawable should stay on the screen 
	 *  before being replaced by another or deleted
	 */
	int _delay;
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
	Sprite(byte *raw_data, uint16 width, uint16 height, int x, int y, bool columnwise);
	
	Sprite(byte *sprite_data, uint16 length, int x, int y, bool columnwise); 

	~Sprite();

	void draw(Surface *surface, bool markDirty = true) const;
	void drawScaled(Surface *surface, bool markDirty = true) const;
	
	void setMirrorOn();
	void setMirrorOff();

	Common::Rect getRect(bool scaled = true) const;

	const byte *getBuffer() const { return _data; }

private:
	byte *_data;	//!< Pointer to a buffer containing raw sprite data (row-wise)
	bool _mirror;
};

class Text : public Drawable {
	
public:
	Text(const Common::String &str, Font *font, byte fontColour, 
		int x, int y, uint spacing = 0);
	~Text();
	
	void setText(const Common::String &str);
	void setColour(byte fontColour);
	void setSpacing(uint spacing);
	
	void draw(Surface *surface, bool markDirty = true) const;

	Common::Rect getRect() const;

private:
	byte *_text;
	uint _length;
	uint8 _colour;
	uint _spacing;
	Font *_font;
};
	
} // End of namespace Draci

#endif // DRACI_SPRITE_H
