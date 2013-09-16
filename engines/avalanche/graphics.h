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

#ifndef AVALANCHE_GRAPHICS_H
#define AVALANCHE_GRAPHICS_H

#include "common/file.h"

#include "graphics/surface.h"
#include "common/rect.h"

namespace Avalanche {
class AvalancheEngine;

typedef byte FontType[256][16]; // raw font type

typedef byte ManiType[2049]; // manitype = array[5..2053] of byte;
// Be aware!!!

typedef byte SilType[51][11]; // 35, 4

class SpriteInfo {
public:
	byte _xWidth;
	byte _xLength, _yLength;
	ManiType *_mani[24];
	SilType *_sil[24];
	uint16 _size; // The size of one picture.
};

class Graphics {
public:
	static const int16 kScreenWidth = 640;
	static const int16 kScreenHeight = 200;
	static const uint16 kBackgroundWidth = kScreenWidth;
	static const byte kBackgroundHeight = 8 * 12080 / kScreenWidth; // With 640 width it's 151.
	// The 8 = number of bits in a byte, and 12080 comes from Lucerna::load().

	::Graphics::Surface _surface;
	::Graphics::Surface _background;
	::Graphics::Surface _magics; // Lucerna::draw_also_lines() draws the "magical" lines here. Further information: https://github.com/urukgit/avalot/wiki/Also
	::Graphics::Surface _scrolls;

	Graphics(AvalancheEngine *vm);
	~Graphics();
	void init();
	void fleshColors();

	// Taken from Free Pascal's Procedure InternalEllipseDefault. Used to replace Pascal's procedure arc.
	// Returns the end point of the arc. (Needed in Lucerna::lucerna_clock().)
	// TODO: Make it more accurate later.
	Common::Point drawArc(::Graphics::Surface &surface, int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, byte color);

	void drawPieSlice(::Graphics::Surface &surface, int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, byte color);
	void drawTriangle(::Graphics::Surface &surface, Common::Point *p, byte color);
	void drawText(::Graphics::Surface &surface, const Common::String &text, FontType font, byte fontHeight, int16 x, int16 y, byte color);

	// The caller has to .free() the returned Surfaces!!!
	// Further information about these two: http://www.shikadi.net/moddingwiki/Raw_EGA_data
	::Graphics::Surface loadPictureGraphic(Common::File &file); // Reads Graphic-planar EGA data.
	::Graphics::Surface loadPictureRow(Common::File &file, uint16 width, uint16 height); // Reads Row-planar EGA data.

	void drawSprite(const SpriteInfo &sprite, byte picnum, int16 x, int16 y);
	void drawPicture(::Graphics::Surface &target, const ::Graphics::Surface &picture, uint16 destX, uint16 destY); // Can't call .free() here. See Lucerna::showscore() for example.
	void refreshScreen();
	void refreshBackground();

private:
	AvalancheEngine *_vm;

	static const byte kEgaPaletteIndex[16];

	byte _egaPalette[64][3];
	::Graphics::Surface _screen; // Only used in refreshScreen() to make it more optimized. (No recreation of it at every call of the function.)
};

} // End of namespace Avalanche

#endif // AVALANCHE_GRAPHICS_H
