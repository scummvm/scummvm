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

#include "math.h"

#include "avalanche/avalanche.h"
#include "avalanche/graphics.h"

#include "common/system.h"
#include "common/rect.h"

#include "engines/util.h"

#include "graphics/palette.h"

namespace Avalanche {

const byte Graphics::kEgaPaletteIndex[16] = {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63};

Graphics::Graphics(AvalancheEngine *vm) {
	_vm = vm;
}

Graphics::~Graphics() {
	_surface.free();
	_magics.free();
	_background.free();
	_screen.free();
	_scrolls.free();
}

void Graphics::init() {
	initGraphics(kScreenWidth, kScreenHeight * 2, true); // Doubling the height.

	for (int i = 0; i < 64; ++i) {
		_egaPalette[i][0] = (i >> 2 & 1) * 0xaa + (i >> 5 & 1) * 0x55;
		_egaPalette[i][1] = (i >> 1 & 1) * 0xaa + (i >> 4 & 1) * 0x55;
		_egaPalette[i][2] = (i      & 1) * 0xaa + (i >> 3 & 1) * 0x55;
	}

	for (byte i = 0; i < 16; i++)
		g_system->getPaletteManager()->setPalette(_egaPalette[kEgaPaletteIndex[i]], i, 1);

	_surface.create(kScreenWidth, kScreenHeight, ::Graphics::PixelFormat::createFormatCLUT8());
	_magics.create(kScreenWidth, kScreenHeight, ::Graphics::PixelFormat::createFormatCLUT8());
	_screen.create(kScreenWidth, kScreenHeight * 2, ::Graphics::PixelFormat::createFormatCLUT8());
	_scrolls.create(kScreenWidth, kScreenHeight, ::Graphics::PixelFormat::createFormatCLUT8());
}

void Graphics::fleshColors()
{
	g_system->getPaletteManager()->setPalette(_egaPalette[39], 13, 1);
	g_system->getPaletteManager()->setPalette(_egaPalette[28], 5, 1);
}

Common::Point Graphics::drawArc(::Graphics::Surface &surface, int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, byte color) {
	Common::Point endPoint;
	const double pi = 3.14;
	const double convfac = pi / 180.0;

	int32 xRadius = radius;
	int32 yRadius = radius * kScreenWidth / (8 * kScreenHeight); // Just don't ask why...

	if (xRadius == 0)
		xRadius++;
	if (yRadius == 0)
		yRadius++;

	// Check for an ellipse with negligable x and y radius.
	if ((xRadius <= 1) && (yRadius <= 1)) 
	{
		*(byte *)_scrolls.getBasePtr(x, y) = color;
		endPoint.x = x;
		endPoint.y = y;
		return endPoint;
	}

	// Check if valid angles.
	stAngle = stAngle % 361;
	endAngle = endAngle % 361;

	// If impossible angles, then swap them! 
	if (endAngle < stAngle) 
	{
		uint16 tmpAngle=endAngle;
		endAngle=stAngle;
		stAngle=tmpAngle;
	}

	// Approximate the number of pixels required by using the circumference equation of an ellipse.                                              
	uint16 numOfPixels=floor(sqrt(3.0)*sqrt(pow(double(xRadius), 2)+pow(double(yRadius), 2)) + 0.5);

	// Calculate the angle precision required.
	double delta = 90.0 / numOfPixels;

	// Always just go over the first 90 degrees. Could be optimized a   
	// bit if startAngle and endAngle lie in the same quadrant, left as an 
	// exercise for the reader. :)                             
	double j = 0;

	// Calculate stop position, go 1 further than 90 because otherwise 1 pixel is sometimes not drawn.                            
	uint16 deltaEnd = 91;

	// Set the end point.
	double tempTerm = endAngle * convfac;
	endPoint.x = floor(xRadius * cos(tempTerm) + 0.5) + x;
	endPoint.y = floor(yRadius * sin(tempTerm + pi) + 0.5) + y;

	// Calculate points. 
	int16 xNext = xRadius;
	int16 yNext = 0;
	do {
		int16 xTemp = xNext;
		int16 yTemp = yNext;
		// This is used by both sin and cos.
		tempTerm = (j + delta) * convfac;
		
		xNext = floor(xRadius*cos(tempTerm) + 0.5);
		yNext = floor(yRadius*sin(tempTerm + pi) + 0.5);

		int16 xp = x + xTemp;
		int16 xm = x - xTemp;
		int16 yp = y + yTemp;
		int16 ym = y - yTemp;

		if ((j >= stAngle) && (j <= endAngle)) 
			*(byte *)_scrolls.getBasePtr(xp,yp) = color;

		if (((180-j) >= stAngle) && ((180-j) <= endAngle)) 
			*(byte *)_scrolls.getBasePtr(xm,yp) = color;

		if (((j+180) >= stAngle) && ((j+180) <= endAngle)) 
			*(byte *)_scrolls.getBasePtr(xm,ym) = color;

		if (((360-j) >= stAngle) && ((360-j) <= endAngle)) 
			*(byte *)_scrolls.getBasePtr(xp,ym) = color;
		
		j += delta;
	} while (j <= deltaEnd);

	return endPoint;
}

void Graphics::drawPieSlice(::Graphics::Surface &surface, int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, byte color) {
	while (radius > 0)
		drawArc(surface, x, y, stAngle, endAngle, radius--, color);
}

void Graphics::drawTriangle(::Graphics::Surface &surface, Common::Point *p, byte color) {
	// Draw the borders with a marking color.
	_scrolls.drawLine(p[0].x, p[0].y, p[1].x, p[1].y, 255);
	_scrolls.drawLine(p[1].x, p[1].y, p[2].x, p[2].y, 255);
	_scrolls.drawLine(p[2].x, p[2].y, p[0].x, p[0].y, 255);

	// Get the top and the bottom of the triangle.
	uint16 maxY = p[0].y, minY = p[0].y;
	for (byte i = 1; i < 3; i++) {
		if (p[i].y < minY)
			minY = p[i].y;
		if (p[i].y > maxY)
			maxY = p[i].y;
	}

	// Fill the triangle.
	for (uint16 y = minY; y <= maxY; y++) {
		uint16 x = 0;
		while (*(byte *)_scrolls.getBasePtr(x, y) != 255)
			x++;
		uint16 minX = x;
		uint16 maxX = x;
		x++;
		while ((*(byte *)_scrolls.getBasePtr(x, y) != 255) && (x != 639))
			x++;
		if (x != 639)
			maxX = x;
		if (minX != maxX)
			_scrolls.drawLine(minX, y, maxX, y, color);
	}

	// Redraw the borders with the actual color.
	_scrolls.drawLine(p[0].x, p[0].y, p[1].x, p[1].y, color);
	_scrolls.drawLine(p[1].x, p[1].y, p[2].x, p[2].y, color);
	_scrolls.drawLine(p[2].x, p[2].y, p[0].x, p[0].y, color);
}

void Graphics::drawText(::Graphics::Surface &surface, const Common::String &text, FontType font, byte fontHeight, int16 x, int16 y, byte color) {
	for (byte i = 0; i < text.size(); i++)
		for (byte j = 0; j < fontHeight; j++) {
			byte pixel = font[(byte)text[i]][j];	
			for (byte bit = 0; bit < 8; bit++) {
				byte pixelBit = (pixel >> bit) & 1;
				if (pixelBit)
					*(byte *)surface.getBasePtr(x + i * 8 + 7 - bit, y + j) = color;
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

void Graphics::drawSprite(const SpriteInfo &sprite, byte picnum, int16 x, int16 y) {
	// First we make the pixels of the spirte blank.
	for (byte j = 0; j < sprite._yLength; j++)
		for (byte i = 0; i < sprite._xLength; i++)
			if (((*sprite._sil[picnum])[j][i / 8] >> ((7 - i % 8)) & 1) == 0)
				*(byte *)_surface.getBasePtr(x + i, y + j) = 0;

	// Then we draw the picture to the blank places.
	uint16 maniPos = 0; // Because the original manitype starts at 5!!! See Graphics.h for definition.

	for (byte j = 0; j < sprite._yLength; j++)
		for (int8 plane = 3; plane >= 0; plane--) // The planes are in the opposite way.
			for (uint16 i = 0; i  < sprite._xLength; i += 8) {
				byte pixel = (*sprite._mani[picnum])[maniPos++];
				for (byte bit = 0; bit < 8; bit++) {
					byte pixelBit = (pixel >> bit) & 1;
					*(byte *)_surface.getBasePtr(x + i + 7 - bit, y + j) += (pixelBit << plane);
				} 
			}
}

void Graphics::drawPicture(::Graphics::Surface &target, ::Graphics::Surface &picture, uint16 destX, uint16 destY) {
	// Copy the picture to the given place on the screen.
	for (uint16 y = 0; y < picture.h; y++)
		for (uint16 x = 0; x < picture.w; x++)
			*(byte *)target.getBasePtr(x + destX, y + destY) = *(byte *)picture.getBasePtr(x, y);		
}

void Graphics::refreshScreen() {
	// These cycles are for doubling the screen height.
	for (uint16 y = 0; y < _screen.h / 2; y++)
		for (uint16 x = 0; x < _screen.w; x++)
			for (byte j = 0; j < 2; j++) 
				*(byte *)_screen.getBasePtr(x, y * 2 + j) = *(byte *)_surface.getBasePtr(x, y);	

	// Now we copy the stretched picture to the screen.
	g_system->copyRectToScreen(_screen.pixels, _screen.pitch, 0, 0, kScreenWidth, kScreenHeight * 2);
	g_system->updateScreen();
}

void Graphics::refreshBackground() {
	_vm->_graphics->drawPicture(_vm->_graphics->_surface, _vm->_graphics->_background, 0, 10);
}

} // End of namespace Avalanche
