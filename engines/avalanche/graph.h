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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* Replacement class for the Graph unit from Pascal. */

#ifndef GRAPH_H
#define GRAPH_H

#include "common/file.h"

#include "graphics/surface.h"

namespace Avalanche {
class AvalancheEngine;

class Graph {
public:
	static const int16 _screenWidth = 640;
	static const int16 _screenHeight = 200;

	

	void setParent(AvalancheEngine *vm);

	void init();

	~Graph();

	byte *getPixel(int16 x, int16 y);

	void setPixel(int16 x, int16 y, byte color);

	void drawBar(int16 x1, int16 y1, int16 x2, int16 y2, int16 color);

	// Must free the returened pointer!!!
	Graphics::Surface *readImage(const byte *source);

	void copySurface(const Graphics::Surface &source, uint16 destX, uint16 destY);

	void refreshScreen();

private:
	AvalancheEngine *_vm;

	Graphics::Surface _surface;

	static const byte _egaPaletteIndex[16];

	byte _egaPalette[64][3];
};

} // End of namespace Avalanche

#endif // GRAPH_H
