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
#include "avalanche/graphics.h"

#include "common/system.h"

#include "engines/util.h"

#include "graphics/palette.h"
#include "common/rect.h"

namespace Avalanche {

const byte Graphics::_egaPaletteIndex[16] = {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63};



Graphics::Graphics(AvalancheEngine *vm) {
	_vm = vm;
}

void Graphics::init() {
	initGraphics(kScreenWidth, kScreenHeight * 2, true); // Doubling the height.

	for (int i = 0; i < 64; ++i) {
		_egaPalette[i][0] = (i >> 2 & 1) * 0xaa + (i >> 5 & 1) * 0x55;
		_egaPalette[i][1] = (i >> 1 & 1) * 0xaa + (i >> 4 & 1) * 0x55;
		_egaPalette[i][2] = (i      & 1) * 0xaa + (i >> 3 & 1) * 0x55;
	}

	for (byte i = 0; i < 16; i++)
		g_system->getPaletteManager()->setPalette(_egaPalette[_egaPaletteIndex[i]], i, 1);

	_surface.create(kScreenWidth, kScreenHeight, ::Graphics::PixelFormat::createFormatCLUT8());
}

Graphics::~Graphics() {
	_surface.free();
}


void Graphics::flesh_colours()
{
	g_system->getPaletteManager()->setPalette(_egaPalette[39], 13, 1);
	g_system->getPaletteManager()->setPalette(_egaPalette[28], 5, 1);
}


byte *Graphics::getPixel(int16 x, int16 y) {
	return (byte *)_surface.getBasePtr(x, y);
}

void Graphics::drawFrame(int16 x1, int16 y1, int16 x2, int16 y2, int16 color) {
	_surface.frameRect(Common::Rect(x1, y1, x2, y2), color);
}

void Graphics::drawBar(int16 x1, int16 y1, int16 x2, int16 y2, int16 color) {
	_surface.fillRect(Common::Rect(x1, y1, x2, y2), color);
}

void Graphics::drawSprite(const SpriteInfo &sprite, byte picnum, int16 x, int16 y) {

	/* First we make the pixels of the spirte blank. */
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

	/* Then we draw the picture to the blank places. */
	uint16 i = 0; // Because the original siltype starts at 5!!! See Graph.h for definition.

	for (byte qay = 0; qay < sprite.yl; qay++)
		for (int8 plane = 3; plane >= 0; plane--) // The planes are in the opposite way.
			for (uint16 qax = 0; qax  < sprite.xl; qax += 8) {
				byte pixel = (*sprite.mani[picnum])[i++];
				for (byte bit = 0; bit < 8; bit++) {
					byte pixelBit = (pixel >> bit) & 1;
					*getPixel(x + qax + 7 - bit, y + qay) += (pixelBit << plane);
				} 
			}
}

::Graphics::Surface Graphics::loadPictureGraphic(Common::File &file) {
	// This function mimics Pascal's getimage().
	// The height and the width are stored in 2-2 bytes. We have to add 1 to each because Pascal stores the value of them -1.
	uint16 pictureWidth = file.readUint16LE() + 1;
	uint16 pictureHeight = file.readUint16LE() + 1;

	::Graphics::Surface picture; // We make a Surface object for the picture itself.

	picture.create(pictureWidth, pictureHeight, ::Graphics::PixelFormat::createFormatCLUT8());

	// Produce the picture. We read it in row-by-row, and every row has 4 planes.
	for (byte y = 0; y < pictureHeight; y++)
		for (int8 plane = 3; plane >= 0; plane--) // The planes are in the opposite way.
			for (uint16 x = 0; x < pictureWidth; x += 8) {
				byte pixel = file.readByte();
				for (byte bit = 0; bit < 8; bit++) {
					byte pixelBit = (pixel >> bit) & 1;
					*(byte *)picture.getBasePtr(x + 7 - bit, y) += (pixelBit << plane);
				} 
			}

	return picture;
}

::Graphics::Surface Graphics::loadPictureRow(Common::File &file, uint16 width, uint16 height) {
	// This function is our own creation, very much like the one above. The main differences are that
	// we don't read the width and the height from the file, the planes are in a different order
	// and we read the picture plane-by-plane.

	::Graphics::Surface picture;

	picture.create(width, height, ::Graphics::PixelFormat::createFormatCLUT8());

	for (byte plane = 0; plane < 4; plane++)
		for (uint16 y = 0; y < height; y++)
			for (uint16 x = 0; x < width; x += 8) {
				byte pixel = file.readByte();
				for (byte i = 0; i < 8; i++) {
					byte pixelBit = (pixel >> i) & 1;
					*(byte *)picture.getBasePtr(x + 7 - i, y) += (pixelBit << plane);
				}	
			}

	return picture;
}

void Graphics::drawPicture(const ::Graphics::Surface &picture, uint16 destX, uint16 destY) {
	// Copy the picture to the given place on the screen.
	for (uint16 y = 0; y < picture.h; y++)
		for (uint16 x = 0; x < picture.w; x++)
			*getPixel(x + destX, y + destY) = *(byte *)picture.getBasePtr(x, y);		
}

void Graphics::refreshScreen() {
	// These cycles are for doubling the screen height.
	::Graphics::Surface picture;
	picture.create(kScreenWidth, kScreenHeight * 2, ::Graphics::PixelFormat::createFormatCLUT8());
	for (uint16 y = 0; y < picture.h / 2; y++)
		for (uint16 x = 0; x < picture.w; x++)
			for (byte j = 0; j < 2; j++) 
				*(byte *)picture.getBasePtr(x, y * 2 + j) = *(byte *)_surface.getBasePtr(x, y);	

	// Now we copy the stretched picture to the screen.
	g_system->copyRectToScreen(picture.pixels, picture.pitch, 0, 0, kScreenWidth, kScreenHeight * 2);
	g_system->updateScreen();

	picture.free();
}


} // End of namespace Avalanche
