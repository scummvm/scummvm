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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common/file.h"

#include "graphics/surface.h"

namespace Avalanche {
class AvalancheEngine;

typedef byte manitype[2049]; // manitype = array[5..2053] of byte; 
// Be aware!!!

typedef byte siltype[51][11]; /* 35, 4 */

class SpriteInfo {
public:
	byte xw; /* x-width in bytes */
	byte xl, yl; /* x & y lengths of pictures */

	manitype *mani[24];
	siltype *sil[24];

	uint16 size; /* the size of one picture */
};


class Graphics {
public:
	static const int16 kScreenWidth = 640;
	static const int16 kScreenHeight = 200;

	::Graphics::Surface _background;

	::Graphics::Surface _magics;
	// Lucerna::draw_also_lines() draws the "magical" lines here.
	// Further information: https://github.com/urukgit/avalot/wiki/Also
	
	::Graphics::Surface _screen;




	Graphics(AvalancheEngine *vm);

	void init();

	~Graphics();

	void flesh_colours();

	byte *getPixel(int16 x, int16 y);

	void drawFrame(int16 x1, int16 y1, int16 x2, int16 y2, int16 color);

	void drawBar(int16 x1, int16 y1, int16 x2, int16 y2, int16 color);

	void drawSprite(const SpriteInfo &sprite, byte picnum, int16 x, int16 y);


	// The caller has to .free() the returned Surfaces!!!
	::Graphics::Surface loadPictureGraphic(Common::File &file); // Reads Graphic-planar EGA data.

	::Graphics::Surface loadPictureRow(Common::File &file, uint16 width, uint16 height); // Reads Row-planar EGA data.
	// Further information about these two: http://www.shikadi.net/moddingwiki/Raw_EGA_data

	void drawPicture(const ::Graphics::Surface &picture, uint16 destX, uint16 destY); // Can't call .free() here. See Lucerna::showscore() for example.

	void refreshScreen();

private:
	AvalancheEngine *_vm;

	::Graphics::Surface _surface;

	static const byte _egaPaletteIndex[16];

	byte _egaPalette[64][3];
};

} // End of namespace Avalanche

#endif // GRAPHICS_H
