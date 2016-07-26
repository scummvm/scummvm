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

namespace Common {
	class File;
}

namespace Lab {

class LabEngine;

class Image {
	LabEngine *_vm;

public:
	uint16 _width;
	uint16 _height;
	byte *_imageData;

	Image(LabEngine *vm) : _width(0), _height(0), _imageData(nullptr), _vm(vm), _autoFree(true) {}
	Image(int w, int h, byte *d, LabEngine *vm, bool autoFree = true) : _width(w), _height(h), _imageData(d), _vm(vm), _autoFree(autoFree) {}
	Image(Common::File *s, LabEngine *vm);
	~Image();

	void setData(byte *d, bool autoFree = true);

	/**
	 * Draws an image to the screen.
	 */
	void drawImage(uint16 x, uint16 y);

	/**
	 * Draws an image to the screen with transparency.
	 */
	void drawMaskImage(uint16 x, uint16 y);

	/**
	 * Reads an image from the screen.
	 */
	void readScreenImage(uint16 x, uint16 y);

	/**
	 * Blits a piece of one image to another.
	 */
	void blitBitmap(uint16 srcX, uint16 srcY, Image *imgDest, uint16 destX, uint16 destY, uint16 width, uint16 height, byte masked);

private:
	bool _autoFree; ///< Free _imageData in destructor?
};

} // End of namespace Lab

#endif // LAB_IMAGE_H
