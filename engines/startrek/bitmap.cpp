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
 */

#include "startrek/bitmap.h"
#include "startrek/startrek.h"

namespace StarTrek {

Bitmap::Bitmap(SharedPtr<FileStream> stream) {
	xoffset = stream->readUint16();
	yoffset = stream->readUint16();
	width = stream->readUint16();
	height = stream->readUint16();

	pixels = new byte[width*height];
	stream->read(pixels, width*height);
}

Bitmap::Bitmap(int w, int h) : width(w), height(h), xoffset(0), yoffset(0) {
	pixels = new byte[width*height];
}

Bitmap::~Bitmap() {
	delete[] pixels;
}


TextBitmap::TextBitmap(int w, int h) {
	width = w;
	height = h;
	// Width and Height are the total dimensions. Since each character takes 8 pixels in
	// each dimension, the "pixels" array (which actually stores character indices) must
	// be smaller.
	pixels = new byte[width/8*height/8];
}

}
