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

#ifndef TITANIC_IMAGE_H
#define TITANIC_IMAGE_H

#include "common/scummsys.h"
#include "common/array.h"

namespace Titanic {

struct BITMAPINFOHEADER {
	int _biSize;
	int _biWidth;
	int _biHeight;
	int _biPlanes;
	int _biBitCount;
	bool _biCompression;
	int _biSizeImage;
	int _biXPelsPerMeter;
	int _biYPelsPerMeter;
	int _biCirUsed;
	int _biClrImportant;

	BITMAPINFOHEADER();
};

struct RGBQuad {
	byte _rgbRed;
	byte _rgbGreen;
	byte _rgbBlue;
	byte _rgbReserved;

	RGBQuad();
};

struct tagBITMAPINFO {
	BITMAPINFOHEADER _bmiHeader;
	RGBQuad _bmiColors[256];
};

class Image {
public:
	tagBITMAPINFO *_bitmapInfo;
	byte *_bits;
	bool _flag;
public:
	Image();

	virtual void proc6();
	virtual void set(int width, int height);
	virtual void proc8();
	virtual bool loadResource(const Common::String &name);
	virtual void proc10();
	virtual void draw();
};

} // End of namespace Titanic

#endif /* TITANIC_IMAGE_H */
