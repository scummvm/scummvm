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

#include "common/stream.h"

#include "draci/draci.h"
#include "draci/sprite.h"
#include "draci/font.h"

namespace Draci {

/**
 *  @brief Transforms an image from column-wise to row-wise
 *	@param img pointer to the buffer containing the image data
 *		   width width of the image in the buffer
 *		   height height of the image in the buffer
 */
static void transformToRows(byte *img, uint16 width, uint16 height) {
	byte *buf = new byte[width * height];
	byte *tmp = buf;
	memcpy(buf, img, width * height);
	
	for (uint16 i = 0; i < width; ++i) {
		for (uint16 j = 0; j < height; ++j) {
			img[j * width + i] = *tmp++;
		}
	}
	
	delete[] buf;
}

/**
 *  Constructor for loading sprites from a raw data buffer, one byte per pixel.
 */ 
Sprite::Sprite(byte *raw_data, uint16 width, uint16 height, uint x, uint y, 
			   uint z, bool columnwise) : _data(NULL) {
	 _width = width;
	 _height = height;
	 _x = x;
	 _y = y;
	 _z = z;
	
	_data = new byte[width * height];
	
	memcpy(_data, raw_data, width * height);

	// If the sprite is stored column-wise, transform it to row-wise
	if (columnwise) {
		transformToRows(_data, width, height);
	}	
}


/**
 *  Constructor for loading sprites from a sprite-formatted buffer, one byte per 
 *	pixel.
 */
Sprite::Sprite(byte *sprite_data, uint16 length, uint x, uint y, uint z,
			   bool columnwise) : _data(NULL) {
	 _x = x;
	 _y = y;
	 _z = z;
	
	Common::MemoryReadStream reader(sprite_data, length);

	_width = reader.readUint16LE();
	_height = reader.readUint16LE();

	_data = new byte[_width * _height];

	reader.read(_data, _width * _height);

	// If the sprite is stored column-wise, transform it to row-wise
	if (columnwise) {
		transformToRows(_data, _width, _height);
	}		
}

Sprite::~Sprite() { 
	delete[] _data;
}

/**
 *  @brief Draws the sprite to a Draci::Surface
 *	@param surface Pointer to a Draci::Surface
 *
 *  Draws the sprite to a Draci::Surface and marks its rectangle on the surface as dirty.
 */
void Sprite::draw(Surface *surface) const { 
	byte *dst = (byte *)surface->getBasePtr(_x, _y);
	byte *src = _data;	
	
	// Blit the sprite to the surface
	for (unsigned int i = 0; i < _height; ++i) {
		for(unsigned int j = 0; j < _width; ++j, ++src) {

			// Don't blit if the pixel is transparent on the target surface
			if (*src != surface->getTransparentColour())			
				dst[j] = *src;
		}
		
		dst += surface->pitch;
	}

	// Mark the sprite's rectangle dirty
	Common::Rect r(_x, _y, _x + _width, _y + _height);
	surface->markDirtyRect(r);
}

Text::Text(const Common::String &str, Font *font, byte fontColour, 
				uint x, uint y, uint z, uint spacing) {
	uint len = str.size();
	_length = len;
	
	_x = x;
	_y = y;
	_z = z;
	
	_text = new byte[len];
	memcpy(_text, str.c_str(), len);
	
	_spacing = spacing;
	_colour = fontColour;
	
	_font = font;

	_width = _font->getStringWidth(str, _spacing);
	_height = _font->getFontHeight();
} 

Text::~Text() {
	delete[] _text;
}

void Text::setText(const Common::String &str) {
	delete[] _text;
	
	uint len = str.size();
	_length = len;

	_width = _font->getStringWidth(str, _spacing);
	_height = _font->getFontHeight();

	 _text = new byte[len];
	memcpy(_text, str.c_str(), len);
}

void Text::setColour(byte fontColour) {
	_colour = fontColour;
}

void Text::draw(Surface *surface) const {
	_font->setColour(_colour);
	_font->drawString(surface, _text, _length, _x, _y, _spacing);
}	
			
} // End of namespace Draci	
		
 
