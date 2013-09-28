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

#include "avalanche/avalanche.h"
#include "avalanche/graphics.h"

#include "common/system.h"
#include "common/rect.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "math.h"

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

	for (int i = 0; i < 10; i++)
		_digits[i].free();
	for (int i = 0; i < 9; i++)
		_directions[i].free();
}

void Graphics::init() {
	initGraphics(kScreenWidth, kScreenHeight * 2, true); // Doubling the height.

	for (int i = 0; i < 64; ++i) {
		_egaPalette[i][0] = (i >> 2 & 1) * 0xaa + (i >> 5 & 1) * 0x55;
		_egaPalette[i][1] = (i >> 1 & 1) * 0xaa + (i >> 4 & 1) * 0x55;
		_egaPalette[i][2] = (i      & 1) * 0xaa + (i >> 3 & 1) * 0x55;
	}

	for (int i = 0; i < 16; i++)
		g_system->getPaletteManager()->setPalette(_egaPalette[kEgaPaletteIndex[i]], i, 1);

	_surface.create(kScreenWidth, kScreenHeight, ::Graphics::PixelFormat::createFormatCLUT8());
	_magics.create(kScreenWidth, kScreenHeight, ::Graphics::PixelFormat::createFormatCLUT8());
	_screen.create(kScreenWidth, kScreenHeight * 2, ::Graphics::PixelFormat::createFormatCLUT8());
	_scrolls.create(kScreenWidth, kScreenHeight, ::Graphics::PixelFormat::createFormatCLUT8());
}

void Graphics::loadDigits(Common::File &file) {   // Load the scoring digits & rwlites
	const byte digitsize = 134;
	const byte rwlitesize = 126;

	if (!file.open("digit.avd"))
		error("AVALANCHE: File not found: digit.avd");

	for (int i = 0; i < 10; i++) {
		file.seek(i * digitsize);
		_digits[i] = loadPictureGraphic(file);
	}

	for (int i = 0; i < 9; i++) {
		file.seek(10 * digitsize + i * rwlitesize);
		_directions[i] = loadPictureGraphic(file);
	}

	file.close();
}

void Graphics::loadMouse(byte which) {
	if (which == _vm->_currentMouse)
		return;

	_vm->_currentMouse = which;

	Common::File f;
	if (!f.open("mice.avd"))
		error("AVALANCHE: Gyro: File not found: mice.avd");

	::Graphics::Surface cursor;
	cursor.create(16, 32, ::Graphics::PixelFormat::createFormatCLUT8());
	cursor.fillRect(Common::Rect(0, 0, 16, 32), 255);


	// The AND mask.
	f.seek(kMouseSize * 2 * which + 134);

	::Graphics::Surface mask = loadPictureGraphic(f);

	for (int j = 0; j < mask.h; j++) {
		for (int i = 0; i < mask.w; i++) {
			byte pixel = *(byte *)mask.getBasePtr(i, j);
			if (pixel == 0) {
				*(byte *)cursor.getBasePtr(i, j * 2    ) = 0;
				*(byte *)cursor.getBasePtr(i, j * 2 + 1) = 0;
			}
		}
	}

	mask.free();

	// The OR mask.
	f.seek(kMouseSize * 2 * which + 134 * 2);

	mask = loadPictureGraphic(f);

	for (int j = 0; j < mask.h; j++) {
		for (int i = 0; i < mask.w; i++) {
			byte pixel = *(byte *)mask.getBasePtr(i, j);
			if (pixel != 0) {
				*(byte *)cursor.getBasePtr(i, j * 2    ) = pixel;
				*(byte *)cursor.getBasePtr(i, j * 2 + 1) = pixel;
			}
		}
	}

	mask.free();
	f.close();

	CursorMan.replaceCursor(cursor.getPixels(), 16, 32, AvalancheEngine::kMouseHotSpots[which]._horizontal, AvalancheEngine::kMouseHotSpots[which]._vertical * 2, 255, false);
	cursor.free();
}

void Graphics::drawThinkPic(Common::String filename, int id) {
	static const int16 kPicSize = 966;
	Common::File file;
	if (!file.open(filename))
		error("drawThinkPic(): File not found: %s", filename.c_str());

	file.seek(id * kPicSize + 65);
	::Graphics::Surface picture = loadPictureGraphic(file);
	drawPicture(_surface, picture, 205, 170);

	picture.free();
	file.close();
}

void Graphics::drawToolbar() {
	Common::File file;
	if (!file.open("useful.avd"))
		error("drawToolbar(): File not found: useful.avd");

	file.seek(40);

	CursorMan.showMouse(false);
	::Graphics::Surface picture = loadPictureGraphic(file);
	drawPicture(_surface, picture, 5, 169);
	CursorMan.showMouse(true);

	picture.free();
	file.close();
}

void Graphics::fleshColors() {
	g_system->getPaletteManager()->setPalette(_egaPalette[39], 13, 1);
	g_system->getPaletteManager()->setPalette(_egaPalette[28], 5, 1);
}

Common::Point Graphics::drawArc(::Graphics::Surface &surface, int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, Color color) {
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
	if ((xRadius <= 1) && (yRadius <= 1)) {
		*(byte *)_scrolls.getBasePtr(x, y) = color;
		endPoint.x = x;
		endPoint.y = y;
		return endPoint;
	}

	// Check if valid angles.
	stAngle = stAngle % 361;
	endAngle = endAngle % 361;

	// If impossible angles, then swap them!
	if (endAngle < stAngle) {
		uint16 tmpAngle=endAngle;
		endAngle=stAngle;
		stAngle=tmpAngle;
	}

	// Approximate the number of pixels required by using the circumference equation of an ellipse.
	uint16 numOfPixels = (uint16)floor(sqrt(3.0) * sqrt(pow(double(xRadius), 2) + pow(double(yRadius), 2)) + 0.5);

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
	endPoint.x = (int16)floor(xRadius * cos(tempTerm) + 0.5) + x;
	endPoint.y = (int16)floor(yRadius * sin(tempTerm + pi) + 0.5) + y;

	// Calculate points.
	int16 xNext = xRadius;
	int16 yNext = 0;
	do {
		int16 xTemp = xNext;
		int16 yTemp = yNext;
		// This is used by both sin and cos.
		tempTerm = (j + delta) * convfac;

		xNext = (int16)floor(xRadius * cos(tempTerm) + 0.5);
		yNext = (int16)floor(yRadius * sin(tempTerm + pi) + 0.5);

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

void Graphics::drawPieSlice(int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, Color color) {
	while (radius > 0)
		drawArc(_scrolls, x, y, stAngle, endAngle, radius--, color);
}

void Graphics::drawTriangle(Common::Point *p, Color color) {
	// Draw the borders with a marking color.
	_scrolls.drawLine(p[0].x, p[0].y, p[1].x, p[1].y, 255);
	_scrolls.drawLine(p[1].x, p[1].y, p[2].x, p[2].y, 255);
	_scrolls.drawLine(p[2].x, p[2].y, p[0].x, p[0].y, 255);

	// Get the top and the bottom of the triangle.
	uint16 maxY = p[0].y, minY = p[0].y;
	for (int i = 1; i < 3; i++) {
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

void Graphics::drawText(::Graphics::Surface &surface, const Common::String text, FontType font, byte fontHeight, int16 x, int16 y, Color color) {
	for (uint i = 0; i < text.size(); i++) {
		for (int j = 0; j < fontHeight; j++) {
			byte pixel = font[(byte)text[i]][j];
			for (int bit = 0; bit < 8; bit++) {
				byte pixelBit = (pixel >> bit) & 1;
				if (pixelBit)
					*(byte *)surface.getBasePtr(x + i * 8 + 7 - bit, y + j) = color;
			}
		}
	}
}

void Graphics::drawNormalText(const Common::String text, FontType font, byte fontHeight, int16 x, int16 y, Color color) {
	_vm->_graphics->drawText(_surface, text, font, fontHeight, x, y, color);
}

void Graphics::drawScrollText(const Common::String text, FontType font, byte fontHeight, int16 x, int16 y, Color color) {
	_vm->_graphics->drawText(_scrolls, text, font, fontHeight, x, y, color);
}

void Graphics::drawDigit(int index, int x, int y) {
	drawPicture(_surface, _digits[index], x, y);
}

void Graphics::drawDirection(int index, int x, int y) {
	drawPicture(_surface, _directions[index], x, y);
}

void Graphics::drawScrollShadow(int16 x1, int16 y1, int16 x2, int16 y2) {
	for (byte i = 0; i < 2; i ++) {
		_scrolls.fillRect(Common::Rect(x1 + i, y1 + i, x1 + i + 1, y2 - i), kColorWhite);
		_scrolls.fillRect(Common::Rect(x1 + i, y1 + i, x2 - i, y1 + i + 1), kColorWhite);

		_scrolls.fillRect(Common::Rect(x2 - i, y1 + i, x2 - i + 1, y2 - i + 1), kColorDarkgray);
		_scrolls.fillRect(Common::Rect(x1 + i, y2 - i, x2 - i, y2 - i + 1), kColorDarkgray);
	}
}

void Graphics::drawShadowBox(int16 x1, int16 y1, int16 x2, int16 y2, Common::String text) {
	CursorMan.showMouse(false);

	drawScrollShadow(x1, y1, x2, y2);

	bool offset = text.size() % 2;
	x1 = (x2 - x1) / 2 + x1 - text.size() / 2 * 8 - offset * 3;
	y1 = (y2 - y1) / 2 + y1 - 4;
	drawScrollText(text, _vm->_font, 8, x1, y1, kColorBlue);
	drawScrollText(Common::String('_'), _vm->_font, 8, x1, y1, kColorBlue);

	CursorMan.showMouse(true);
}

::Graphics::Surface Graphics::loadPictureGraphic(Common::File &file) {
	// This function mimics Pascal's getimage().
	// The height and the width are stored in 2-2 bytes. We have to add 1 to each because Pascal stores the value of them -1.
	uint16 width = file.readUint16LE() + 1;
	uint16 height = file.readUint16LE() + 1;

	::Graphics::Surface picture; // We make a Surface object for the picture itself.
	picture.create(width, height, ::Graphics::PixelFormat::createFormatCLUT8());

	// Produce the picture. We read it in row-by-row, and every row has 4 planes.
	for (int y = 0; y < height; y++) {
		for (int8 plane = 3; plane >= 0; plane--) { // The planes are in the opposite way.
			for (uint16 x = 0; x < width; x += 8) {
				byte pixel = file.readByte();
				for (int bit = 0; bit < 8; bit++) {
					byte pixelBit = (pixel >> bit) & 1;
					if (pixelBit != 0)
						*(byte *)picture.getBasePtr(x + 7 - bit, y) += (pixelBit << plane);
				}
			}
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

	for (int plane = 0; plane < 4; plane++) {
		for (uint16 y = 0; y < height; y++) {
			for (uint16 x = 0; x < width; x += 8) {
				byte pixel = file.readByte();
				for (int i = 0; i < 8; i++) {
					byte pixelBit = (pixel >> i) & 1;
					*(byte *)picture.getBasePtr(x + 7 - i, y) += (pixelBit << plane);
				}
			}
		}
	}

	return picture;
}

void Graphics::clearAlso() {
	_magics.fillRect(Common::Rect(0, 0, 640, 200), 0);
	_magics.frameRect(Common::Rect(0, 45, 640, 161), 15);
}

void Graphics::clearTextBar() {
	_surface.fillRect(Common::Rect(24, 161, 640, 169), kColorBlack); // Black out the line of the text.
}

void Graphics::setAlsoLine(int x1, int y1, int x2, int y2, Color color) {
	_magics.drawLine(x1, y1, x2, y2, color);
}

byte Graphics::getAlsoColor(int x1, int y1, int x2, int y2) {
	byte returnColor = 0;
	for (int16 i = x1; i <= x2; i++) {
		for (int16 j = y1; j <= y2; j++) {
			byte actColor = *(byte *)_vm->_graphics->_magics.getBasePtr(i, j);
			returnColor = MAX(returnColor, actColor);
		}
	}

	return returnColor;
}

void Graphics::drawSprite(const SpriteInfo &sprite, byte picnum, int16 x, int16 y) {
	// First we make the pixels of the sprite blank.
	for (int j = 0; j < sprite._yLength; j++) {
		for (int i = 0; i < sprite._xLength; i++) {
			if (((*sprite._sil[picnum])[j][i / 8] >> ((7 - i % 8)) & 1) == 0)
				*(byte *)_surface.getBasePtr(x + i, y + j) = 0;
		}
	}

	// Then we draw the picture to the blank places.
	uint16 maniPos = 0; // Because the original manitype starts at 5!!! See Graphics.h for definition.

	for (int j = 0; j < sprite._yLength; j++) {
		for (int8 plane = 3; plane >= 0; plane--) { // The planes are in the opposite way.
			for (uint16 i = 0; i  < sprite._xLength; i += 8) {
				byte pixel = (*sprite._mani[picnum])[maniPos++];
				for (int bit = 0; bit < 8; bit++) {
					byte pixelBit = (pixel >> bit) & 1;
					*(byte *)_surface.getBasePtr(x + i + 7 - bit, y + j) += (pixelBit << plane);
				}
			}
		}
	}
}

void Graphics::drawPicture(::Graphics::Surface &target, const ::Graphics::Surface &picture, uint16 destX, uint16 destY) {
	// Copy the picture to the given place on the screen.
	for (uint16 y = 0; y < picture.h; y++) {
		for (uint16 x = 0; x < picture.w; x++)
			*(byte *)target.getBasePtr(x + destX, y + destY) = *(const byte *)picture.getBasePtr(x, y);
	}
}

void Graphics::refreshScreen() {
	// These cycles are for doubling the screen height.
	for (uint16 y = 0; y < _screen.h / 2; y++) {
		for (uint16 x = 0; x < _screen.w; x++) {
			for (int j = 0; j < 2; j++)
				*(byte *)_screen.getBasePtr(x, y * 2 + j) = *(byte *)_surface.getBasePtr(x, y);
		}
	}
	// Now we copy the stretched picture to the screen.
	g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, kScreenWidth, kScreenHeight * 2);
	g_system->updateScreen();
}

void Graphics::loadBackground(Common::File &file) {
	_background = loadPictureRow(file, kBackgroundWidth, kBackgroundHeight);
}

void Graphics::refreshBackground() {
	drawPicture(_surface, _background, 0, 10);
}

void Graphics::zoomOut(int16 x, int16 y) {
	//setlinestyle(dottedln, 0, 1); TODO: Implement it with a dotted line style!!!

	::Graphics::Surface backup;
	backup.copyFrom(_surface);

	for (byte i = 1; i <= 20; i ++) {
		int16 x1 = x - (x / 20) * i;
		int16 y1 = y - ((y - 10) / 20) * i;
		int16 x2 = x + (((639 - x) / 20) * i);
		int16 y2 = y + (((161 - y) / 20) * i);

		_surface.frameRect(Common::Rect(x1, y1, x2, y2), kColorWhite);
		refreshScreen();
		_vm->_system->delayMillis(17);
		_surface.copyFrom(backup);
		refreshScreen();
	}

	backup.free();
}

// Original name background()
void Graphics::setBackgroundColor(Color x) {
	warning("STUB: setBackgroundColor(%d)", x);
}

} // End of namespace Avalanche
