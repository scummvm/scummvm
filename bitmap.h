// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef BITMAP_H
#define BITMAP_H

#include "resource.h"

#include <cstring>

class Bitmap : public Resource {
public:
	// Construct a bitmap from the given data.
	Bitmap(const char *filename, const char *data, int len);
	Bitmap(const char *data, int width, int height, const char *filename);

	void draw() const;

	// Set which image in an animated bitmap to use
	void setNumber(int n) { _currImage = n; }

	int numImages() const { return _numImages; }
	int currentImage() const { return _currImage; }

	int width() const { return _width; }
	int height() const { return _height; }
	int x() const { return _x; }
	int y() const { return _y; }
	void setX(int x) { _x = x; }
	void setY(int y) { _y = y; }

	char *getData() { return _data[_currImage]; }

	char *getFilename() { return _filename; }

	~Bitmap();

//private:
	char **_data;
	int _numImages, _currImage;
	int _width, _height, _x, _y;
	int _format;
	int _numTex;
	void *_texIds;
	bool _hasTransparency;
	char _filename[32];
};

#endif
