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

#include "titanic/string.h"
#include "titanic/simple_file.h"
#include "titanic/video_surface.h"

namespace Titanic {

class CJPEGDecode {
private:
	StdCWadFile _file;
	int _width, _height;
public:
	CJPEGDecode(const CString &name);

	/**
	 * Return the width of the JPEG
	 */
	int getWidth() const { return _width; }

	/**
	 * Return the height of the JPEG
	 */
	int getHeight() const { return _height; }

	/**
	 * Decode the image onto the passed surface
	 */
	void decode(OSVideoSurface &surface);
};

class CTargaDecode {
private:
	StdCWadFile _file;
	int _width, _height;
public:
	CTargaDecode(const CString &name);

	/**
	* Return the width of the JPEG
	*/
	int getWidth() const { return _width; }

	/**
	* Return the height of the JPEG
	*/
	int getHeight() const { return _height; }

	/**
	 * Decode the image onto the passed surface
	 */
	void decode(OSVideoSurface &surface);

};

} // End of namespace Titanic

#endif /* TITANIC_IMAGE_DECODERS_H */
