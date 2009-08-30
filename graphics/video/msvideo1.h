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
 
#ifndef GRAPHICS_MSVIDEO1_H
#define GRAPHICS_MSVIDEO1_H
 
#include "graphics/video/avi_player.h"
#include "graphics/surface.h"
 
namespace Graphics {
 
class MSVideo1Decoder : public Codec {
public:
	MSVideo1Decoder(uint16 width, uint16 height, byte bitsPerPixel);
	~MSVideo1Decoder();

	Surface *decodeImage(Common::SeekableReadStream *stream);

private:
	byte _bitsPerPixel;
	
	Surface *_surface;
	
	void decode8(Common::SeekableReadStream *stream);
	//void decode16(Common::SeekableReadStream *stream);
};

}

#endif
