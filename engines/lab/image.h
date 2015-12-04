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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_IMAGE_H
#define LAB_IMAGE_H

namespace Lab {

class Image {
public:
	uint16 _width;
    uint16 _height;
	byte *_imageData;

    Image() : _width(0), _height(0), _imageData(0) {}
    Image(int w, int h, byte *d) : _width(w), _height(h), _imageData(d) {}
	Image(Common::File *s);

    void drawImage(uint16 x, uint16 y);
    void drawMaskImage(uint16 x, uint16 y);
    void readScreenImage(uint16 x, uint16 y);
	void blitBitmap(uint16 xs, uint16 ys, Image *ImDest, uint16 xd, uint16 yd, uint16 width, uint16 height, byte masked);
};


} // End of namespace Lab

#endif // LAB_H
