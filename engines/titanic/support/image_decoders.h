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

#ifndef TITANIC_IMAGE_DECODERS_H
#define TITANIC_IMAGE_DECODERS_H

#include "image/jpeg.h"
#include "image/tga.h"
#include "titanic/support/string.h"
#include "titanic/support/simple_file.h"
#include "titanic/support/video_surface.h"

namespace Titanic {

class CJPEGDecode : public Image::JPEGDecoder {
public:
	/**
	 * Decode the image file onto the passed surface
	 */
	void decode(OSVideoSurface &surface, const CString &name);
};

class CTargaDecode : public Image::TGADecoder {
public:
	/**
	* Decode the image file onto the passed surface
	*/
	void decode(OSVideoSurface &surface, const CString &name);
};

} // End of namespace Titanic

#endif /* TITANIC_IMAGE_DECODERS_H */
