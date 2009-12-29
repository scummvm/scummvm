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

#ifndef MYST_JPEG_H
#define MYST_JPEG_H

#include "common/scummsys.h"
#include "common/stream.h"

#include "graphics/jpeg.h"
#include "graphics/pixelformat.h"
 
namespace Mohawk {

// Myst JPEG Decoder
// Basically a wrapper around JPEG which converts to RGB

class MystJPEG {
public:
	MystJPEG();
	~MystJPEG() { delete _jpeg; }
	
	Graphics::Surface *decodeImage(Common::SeekableReadStream *stream);
	
private:
	Graphics::PixelFormat _pixelFormat;
	Graphics::JPEG *_jpeg;
};

}

#endif
