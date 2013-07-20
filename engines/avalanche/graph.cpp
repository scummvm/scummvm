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

#include "avalanche/avalanche.h"
#include "avalanche/graph.h"

#include "common/system.h"

#include "engines/util.h"

#include "graphics/palette.h"
#include "common/rect.h"

namespace Avalanche {

const byte Graph::_egaPaletteIndex[16] = {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63};



void Graph::setParent(AvalancheEngine *vm) {
	_vm = vm;
}

void Graph::init() {
	initGraphics(_screenWidth, _screenHeight, true);

	for (int i = 0; i < 64; ++i) {
		_egaPalette[i][0] = (i >> 2 & 1) * 0xaa + (i >> 5 & 1) * 0x55;
		_egaPalette[i][1] = (i >> 1 & 1) * 0xaa + (i >> 4 & 1) * 0x55;
		_egaPalette[i][2] = (i      & 1) * 0xaa + (i >> 3 & 1) * 0x55;
	}

	for (byte i = 0; i < 16; i++)
		g_system->getPaletteManager()->setPalette(_egaPalette[_egaPaletteIndex[i]], i, 1);

	_surface.create(_screenWidth, _screenHeight, Graphics::PixelFormat::createFormatCLUT8());
}

Graph::~Graph() {
	_surface.free();
}

byte *Graph::getPixel(int16 x, int16 y) {
	return (byte *)_surface.getBasePtr(x, y);
}

void Graph::drawBar(int16 x1, int16 y1, int16 x2, int16 y2, int16 color) {
	_surface.fillRect(Common::Rect(x1, y1, x2, y2), color);
}

void Graph::drawSprite(const SpriteInfo &sprite, byte picnum, int16 x, int16 y) {

	for (byte qay = 0; qay < sprite.yl; qay++) {
		byte *mask = new byte[sprite.xl];

		for (byte qax = 0; qax < sprite.xl; qax++) {
			byte count = qax / 8;
			mask[qax] = ((*sprite.sil[picnum])[qay][count] >> ((7 - qax % 8)) & 1);
			if (mask[qax] == 0)
				*getPixel(x + qax, y + qay) = 0;
		}

		delete[] mask;
	}

/*
	for (fv = 5; fv <= sprite.size - 2; fv ++)
		aa[fv] = aa[fv] ^ (*sprite.mani[picnum])[fv];
*/

	warning("STUB: Graph::drawSprite()");
}

void Graph::drawPicture(const byte *source, uint16 destX, uint16 destY) {
	// The height and the width are stored in 2-2 bytes. We have to add 1 to each becouse Pascal stores the value of them -1.
	uint16 pictureWidth = READ_LE_UINT16(source) + 1;
	uint16 pictureHeight = READ_LE_UINT16(source + 2) + 1;

	uint32 i = 4;

	Graphics::Surface picture; // We make a Surface object for the picture itself.

	picture.create(pictureWidth, pictureHeight, Graphics::PixelFormat::createFormatCLUT8());

	// Produce the picture.
	for (byte y = 0; y < pictureHeight; y++)
		for (int8 plane = 3; plane >= 0; plane--) // The planes are in the opposite way.
			for (uint16 x = 0; x < pictureWidth; x += 8) {
				byte pixel = source[i++];
				for (byte bit = 0; bit < 8; bit++) {
					byte pixelBit = (pixel >> bit) & 1;
					*(byte *)picture.getBasePtr(x + 7 - bit, y) += (pixelBit << plane);
				} 
			}

	// Copy the picture to a given place on the screen.
	for (uint16 y = 0; y < picture.h; y++)
		for (uint16 x = 0; x < picture.w; x++)
			*(byte *)_surface.getBasePtr(x + destX, y + destY) = *(byte *)picture.getBasePtr(x, y);		
}

void Graph::refreshScreen() {
	g_system->copyRectToScreen(_surface.pixels, _surface.pitch , 0, 0, _screenWidth, _screenHeight);
	g_system->updateScreen();
}

} // End of namespace Avalanche
