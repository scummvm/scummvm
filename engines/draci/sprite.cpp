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

namespace Draci {

/**
 *  Constructor for loading sprites from a raw data buffer, one byte per pixel.
 */
Sprite::Sprite(byte *raw_data, uint16 width, uint16 height, uint16 x, uint16 y, 
			   bool columnwise) : _width(width), _height(height), _x(x), _y(y), _data(NULL) {
	
	_data = new byte[width * height];
		
	if (!columnwise) {
		memcpy(_data, raw_data, width * height);
	} else {
		for (uint16 i = 0; i < width; ++i) {
			for (uint16 j = 0; j < height; ++j) {
				_data[j * width + i] = *raw_data++;
			}
		}
	}
}

/**
 *  Constructor for loading sprites from a sprite-formatted buffer, one byte per 
 *	pixel.
 */
Sprite::Sprite(byte *sprite_data, uint16 length, uint16 x, uint16 y, 
			   bool columnwise) : _x(x), _y(y), _data(NULL) {

	Common::MemoryReadStream reader(sprite_data, length);

	_width = reader.readUint16LE();
	_height = reader.readUint16LE();

	_data = new byte[_width * _height];
		
	if (!columnwise) {
		reader.read(_data, _width * _height);
	} else {
		for (uint16 i = 0; i < _width; ++i) {
			for (uint16 j = 0; j < _height; ++j) {
				_data[j * _width + i] = reader.readByte();
			}
		}
	}		
}

Sprite::~Sprite() { 
	delete[] _data;
}

void Sprite::draw(Surface *surface) const { 
	byte *dst = (byte *)surface->getBasePtr(_x, _y);
	byte *src = _data;	
	
	for (unsigned int i = 0; i < _height; ++i) {
		for(unsigned int j = 0; j < _width; ++j, ++src) {
			if (*src != surface->getTransparentColour())			
				dst[j] = *src;
		}
		
		dst += surface->pitch;
	}

	Common::Rect r(_x, _y, _x + _width, _y + _height);
	surface->markDirtyRect(r);
}
		
			
} // End of namespace Draci	
		
 
