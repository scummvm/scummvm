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

#include "engines/util.h"
#include "graphics/palette.h"

namespace Avalanche {

const byte GraphicManager::kEgaPaletteIndex[16] = {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63};

const MouseHotspotType GraphicManager::kMouseHotSpots[9] = {
	{8,0},  // 0 - up-arrow
	{0,0},  // 1 - screwdriver
	{15,6}, // 2 - right-arrow
	{0,0},  // 3 - fletch
	{8,7},  // 4 - hourglass
	{4,0},  // 5 - TTHand
	{8,5},  // 6 - Mark's crosshairs
	{8,7},  // 7 - I-beam
	{0,0}   // 8 - question mark
};

GraphicManager::GraphicManager(AvalancheEngine *vm) {
	_vm = vm;
	setDialogColor(kColorBlack, kColorWhite);
}

GraphicManager::~GraphicManager() {
	_surface.free();
	_magics.free();
	_background.free();
	_screen.free();
	_scrolls.free();
	_backup.free();

	for (int i = 0; i < 10; i++)
		_digits[i].free();
	for (int i = 0; i < 9; i++)
		_directions[i].free();
}

void GraphicManager::init() {
	initGraphics(kScreenWidth, kScreenHeight * 2, true); // Doubling the height.

	for (int i = 0; i < 64; ++i) {
		_egaPalette[i][0] = (i >> 2 & 1) * 0xaa + (i >> 5 & 1) * 0x55;
		_egaPalette[i][1] = (i >> 1 & 1) * 0xaa + (i >> 4 & 1) * 0x55;
		_egaPalette[i][2] = (i      & 1) * 0xaa + (i >> 3 & 1) * 0x55;
	}

	for (int i = 0; i < 16; i++)
		g_system->getPaletteManager()->setPalette(_egaPalette[kEgaPaletteIndex[i]], i, 1);

	// Set the "flesh colors":
	g_system->getPaletteManager()->setPalette(_egaPalette[39], 13, 1);
	g_system->getPaletteManager()->setPalette(_egaPalette[28], 5, 1);

	_surface.create(kScreenWidth, kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
	_magics.create(kScreenWidth, kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
	_screen.create(kScreenWidth, kScreenHeight * 2, Graphics::PixelFormat::createFormatCLUT8());
	_scrolls.create(kScreenWidth, kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
}

/**
 * Load the scoring digits & rwlites
 * @remarks	Originally called 'load_digits'
 */
void GraphicManager::loadDigits() {
	const byte digitsize = 134;
	const byte rwlitesize = 126;

	Common::File file;
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

void GraphicManager::loadMouse(byte which) {
	if (which == _vm->_currentMouse)
		return;

	_vm->_currentMouse = which;

	Common::File f;
	if (!f.open("mice.avd"))
		error("AVALANCHE: Gyro: File not found: mice.avd");

	Graphics::Surface cursor;
	cursor.create(16, 32, Graphics::PixelFormat::createFormatCLUT8());
	cursor.fillRect(Common::Rect(0, 0, 16, 32), 255);

	// The AND mask.
	f.seek(kMouseSize * 2 * which + 134);

	Graphics::Surface mask = loadPictureGraphic(f);

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

	CursorMan.replaceCursor(cursor.getPixels(), 16, 32, kMouseHotSpots[which]._horizontal, kMouseHotSpots[which]._vertical * 2, 255, false);
	cursor.free();
}

void GraphicManager::drawThinkPic(Common::String filename, int id) {
	static const int16 picSize = 966;
	Common::File file;
	if (!file.open(filename))
		error("drawThinkPic(): File not found: %s", filename.c_str());

	file.seek(id * picSize + 65);
	Graphics::Surface picture = loadPictureGraphic(file);
	drawPicture(_surface, picture, 205, 170);

	picture.free();
	file.close();
}

void GraphicManager::drawToolbar() {
	Common::File file;
	if (!file.open("useful.avd"))
		error("drawToolbar(): File not found: useful.avd");

	file.seek(40);

	CursorMan.showMouse(false);
	Graphics::Surface picture = loadPictureGraphic(file);
	drawPicture(_surface, picture, 5, 169);
	CursorMan.showMouse(true);

	picture.free();
	file.close();
}

Common::Point GraphicManager::drawArc(Graphics::Surface &surface, int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, Color color) {
	Common::Point endPoint;
	const float convfac = M_PI / 180.0;

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
	float delta = 90.0 / numOfPixels;

	// Always just go over the first 90 degrees. Could be optimized a
	// bit if startAngle and endAngle lie in the same quadrant, left as an
	// exercise for the reader. :)
	float j = 0;

	// Calculate stop position, go 1 further than 90 because otherwise 1 pixel is sometimes not drawn.
	uint16 deltaEnd = 91;

	// Set the end point.
	float tempTerm = endAngle * convfac;
	endPoint.x = (int16)floor(xRadius * cos(tempTerm) + 0.5) + x;
	endPoint.y = (int16)floor(yRadius * sin(tempTerm + M_PI) + 0.5) + y;

	// Calculate points.
	int16 xNext = xRadius;
	int16 yNext = 0;
	do {
		int16 xTemp = xNext;
		int16 yTemp = yNext;
		// This is used by both sin and cos.
		tempTerm = (j + delta) * convfac;

		xNext = (int16)floor(xRadius * cos(tempTerm) + 0.5);
		yNext = (int16)floor(yRadius * sin(tempTerm + M_PI) + 0.5);

		int16 xp = x + xTemp;
		int16 xm = x - xTemp;
		int16 yp = y + yTemp;
		int16 ym = y - yTemp;

		if ((j >= stAngle) && (j <= endAngle))
			*(byte *)_scrolls.getBasePtr(xp, yp) = color;

		if (((180 - j) >= stAngle) && ((180 - j) <= endAngle))
			*(byte *)_scrolls.getBasePtr(xm, yp) = color;

		if (((j + 180) >= stAngle) && ((j + 180) <= endAngle))
			*(byte *)_scrolls.getBasePtr(xm, ym) = color;

		if (((360 - j) >= stAngle) && ((360 - j) <= endAngle))
			*(byte *)_scrolls.getBasePtr(xp, ym) = color;

		j += delta;
	} while (j <= deltaEnd);

	return endPoint;
}

void GraphicManager::drawLine(int x1, int y1, int x2, int y2, int penX, int penY, Color color) {
	_surface.drawThickLine(x1, y1, x2, y2, penX, penY, color);
}

Common::Point GraphicManager::drawScreenArc(int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, Color color) {
	return drawArc(_surface, x, y, stAngle, endAngle, radius, color);
}

void GraphicManager::drawPieSlice(int16 x, int16 y, int16 stAngle, int16 endAngle, uint16 radius, Color color) {
	while (radius > 0)
		drawArc(_scrolls, x, y, stAngle, endAngle, radius--, color);
}

void GraphicManager::drawTriangle(Common::Point *p, Color color) {
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

void GraphicManager::drawText(Graphics::Surface &surface, const Common::String text, FontType font, byte fontHeight, int16 x, int16 y, Color color) {
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

void GraphicManager::drawNormalText(const Common::String text, FontType font, byte fontHeight, int16 x, int16 y, Color color) {
	drawText(_surface, text, font, fontHeight, x, y, color);
}

void GraphicManager::drawScrollText(const Common::String text, FontType font, byte fontHeight, int16 x, int16 y, Color color) {
	drawText(_scrolls, text, font, fontHeight, x, y, color);
}

void GraphicManager::drawDigit(int index, int x, int y) {
	drawPicture(_surface, _digits[index], x, y);
}

void GraphicManager::drawDirection(int index, int x, int y) {
	drawPicture(_surface, _directions[index], x, y);
}

void GraphicManager::drawScrollShadow(int16 x1, int16 y1, int16 x2, int16 y2) {
	for (byte i = 0; i < 2; i ++) {
		_scrolls.fillRect(Common::Rect(x1 + i, y1 + i, x1 + i + 1, y2 - i), kColorWhite);
		_scrolls.fillRect(Common::Rect(x1 + i, y1 + i, x2 - i, y1 + i + 1), kColorWhite);

		_scrolls.fillRect(Common::Rect(x2 - i, y1 + i, x2 - i + 1, y2 - i + 1), kColorDarkgray);
		_scrolls.fillRect(Common::Rect(x1 + i, y2 - i, x2 - i, y2 - i + 1), kColorDarkgray);
	}
}

void GraphicManager::drawShadowBox(int16 x1, int16 y1, int16 x2, int16 y2, Common::String text) {
	CursorMan.showMouse(false);

	drawScrollShadow(x1, y1, x2, y2);

	bool offset = text.size() % 2;
	x1 = (x2 - x1) / 2 + x1 - text.size() / 2 * 8 - offset * 3;
	y1 = (y2 - y1) / 2 + y1 - 4;
	drawScrollText(text, _vm->_font, 8, x1, y1, kColorBlue);
	drawScrollText(Common::String('_'), _vm->_font, 8, x1, y1, kColorBlue);

	CursorMan.showMouse(true);
}

void GraphicManager::drawMenuBar(Color color) {
	_surface.fillRect(Common::Rect(0, 0, 640, 10), color);
}

void GraphicManager::drawMenuBlock(int x1, int y1, int x2, int y2, Color color) {
	_surface.fillRect(Common::Rect(x1, y1, x2, y2), color);
}

void GraphicManager::drawMenuItem(int x1, int y1, int x2, int y2) {
	_surface.fillRect(Common::Rect(x1, y1, x2, y2), kMenuBackgroundColor);
	_surface.frameRect(Common::Rect(x1 - 1, y1 - 1, x2 + 1, y2 + 1), kMenuBorderColor);
}

void GraphicManager::drawSpeedBar(int speed) {
	if (speed == kRun) {
		_surface.drawLine(336, 199, 338, 199, kColorLightblue);
		_surface.drawLine(371, 199, 373, 199, kColorYellow);
	} else {
		_surface.drawLine(371, 199, 373, 199, kColorLightblue);
		_surface.drawLine(336, 199, 338, 199, kColorYellow);
	}
}
void GraphicManager::drawScroll(int mx, int lx, int my, int ly) {
	_scrolls.copyFrom(_surface);

	// The right corners of the scroll.
	drawPieSlice(mx + lx, my - ly, 0, 90, 15, kColorLightgray);
	drawPieSlice(mx + lx, my + ly, 270, 360, 15, kColorLightgray);
	drawArc(_scrolls, mx + lx, my - ly, 0, 90, 15, kColorRed);
	drawArc(_scrolls, mx + lx, my + ly, 270, 360, 15, kColorRed);

	// The body of the scroll.
	_scrolls.fillRect(Common::Rect(mx - lx - 30, my + ly, mx + lx, my + ly + 6), kColorLightgray);
	_scrolls.fillRect(Common::Rect(mx - lx - 30, my - ly - 6, mx + lx, my - ly + 1), kColorLightgray);
	_scrolls.fillRect(Common::Rect(mx - lx - 15, my - ly, mx + lx + 15, my + ly + 1), kColorLightgray);

	// The left corners of the scroll.
	drawPieSlice(mx - lx - 31, my - ly, 0, 180, 15, kColorDarkgray);
	drawArc(_scrolls, mx - lx - 31, my - ly, 0, 180, 15, kColorRed);
	_scrolls.drawLine(mx - lx - 31 - 15, my - ly, mx - lx - 31 + 15, my - ly, kColorRed);
	drawPieSlice(mx - lx - 31, my + ly, 180, 360, 15, kColorDarkgray);
	drawArc(_scrolls, mx - lx - 31, my + ly, 180, 360, 15, kColorRed);
	_scrolls.drawLine(mx - lx - 31 - 15, my + ly, mx - lx - 31 + 15, my + ly, kColorRed);

	// The rear borders of the scroll.
	_scrolls.fillRect(Common::Rect(mx - lx - 30, my - ly - 6, mx + lx, my - ly - 5), kColorRed);
	_scrolls.fillRect(Common::Rect(mx - lx - 30, my + ly + 6, mx + lx, my + ly + 7), kColorRed);
	_scrolls.fillRect(Common::Rect(mx - lx - 15, my - ly, mx - lx - 14, my + ly), kColorRed);
	_scrolls.fillRect(Common::Rect(mx + lx + 15, my - ly, mx + lx + 16, my + ly), kColorRed);
}

void GraphicManager::drawBackgroundSprite(int16 x, int16 y, SpriteType &sprite) {
	drawPicture(_background, sprite._picture, x, y);
}

void GraphicManager::drawDebugLines() {
	if (!_vm->_showDebugLines)
		return;

	for (int i = 0; i < _vm->_lineNum; i++) {
		LineType *curLine = &_vm->_lines[i];
		_surface.drawLine(curLine->_x1, curLine->_y1, curLine->_x2, curLine->_y2, curLine->_color);
	}

	for (int i = 0; i < _vm->_fieldNum; i++) {
		FieldType *curField = &_vm->_fields[i];
		if (curField->_x1 < 640)
			_surface.frameRect(Common::Rect(curField->_x1, curField->_y1, curField->_x2, curField->_y2), kColorLightmagenta);
	}
}

void GraphicManager::drawFilledRectangle(Common::Rect rect, Color color) {
	_surface.fillRect(rect, color);
}

void GraphicManager::drawRectangle(Common::Rect rect, Color color) {
	_surface.frameRect(rect, color);
}

void GraphicManager::nimLoad() {
	Common::File file;
	Common::String filename = "nim.avd";

	if (!file.open(filename))
		error("AVALANCHE: Scrolls: File not found: %s", filename.c_str());

	file.seek(41);

	_nimStone = loadPictureSign(file, 7, 23);
	for (int i = 0; i < 3; i++)
		_nimInitials[i] = loadPictureSign(file, 7, 23);
	_nimLogo = loadPictureSign(file, 30, 37);

	file.close();
}

void GraphicManager::nimDrawStone(int x, int y) {
	drawPicture(_surface, _nimStone, x, y);
}

void GraphicManager::nimDrawInitials() {
	for (int i = 0; i < 3; i++)
		drawPicture(_surface, _nimInitials[i], 0, 75 + i * 35);
}

void GraphicManager::nimDrawLogo() {
	drawPicture(_surface, _nimLogo, 392, 5);
}

void GraphicManager::nimFree() {
	_nimStone.free();
	for (int i = 0; i < 3; i++)
		_nimInitials[i].free();
	_nimLogo.free();
}

/**
 * This function mimics Pascal's getimage().
 */
Graphics::Surface GraphicManager::loadPictureGraphic(Common::File &file) {
	// The height and the width are stored in 2-2 bytes. We have to add 1 to each because Pascal stores the value of them -1.
	uint16 width = file.readUint16LE() + 1;
	uint16 height = file.readUint16LE() + 1;

	Graphics::Surface picture; // We make a Surface object for the picture itself.
	picture.create(width, height, Graphics::PixelFormat::createFormatCLUT8());

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

/**
 * Reads Row-planar EGA data.
 * This function is our own creation, very much like the one above. The main differences are that
 * we don't read the width and the height from the file, the planes are in a different order
 * and we read the picture plane-by-plane.
 */
Graphics::Surface GraphicManager::loadPictureRaw(Common::File &file, uint16 width, uint16 height) {
	Graphics::Surface picture;
	picture.create(width, height, Graphics::PixelFormat::createFormatCLUT8());

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

Graphics::Surface GraphicManager::loadPictureSign(Common::File &file, int xl, int yl) {
	// I know it looks very similar to the other loadPicture methods, but in truth it's the combination of the two.
	uint16 width = xl * 8;
	uint16 height = yl;

	Graphics::Surface picture; // We make a Surface object for the picture itself.
	picture.create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	// Produce the picture. We read it in row-by-row, and every row has 4 planes.
	for (int yy = 0; yy < height; yy++) {
		for (int8 plane = 0; plane < 4; plane++) { // The planes are in the "right" order.
			for (uint16 xx = 0; xx < width; xx += 8) {
				byte pixel = file.readByte();
				for (int bit = 0; bit < 8; bit++) {
					byte pixelBit = (pixel >> bit) & 1;
					if (pixelBit != 0)
						*(byte *)picture.getBasePtr(xx + 7 - bit, yy) += (pixelBit << plane);
				}
			}
		}
	}

	return picture;
}

/**
* Shifts the whole screen down by one line and fills the gap with black.
*/
void GraphicManager::shiftScreen() {
	for (uint16 y = _surface.h - 1; y > 1; y--)
		memcpy(_surface.getBasePtr(0, y), _surface.getBasePtr(0, y - 1), _surface.w);

	_surface.drawLine(0, 0, _surface.w, 0, kColorBlack);
}

void GraphicManager::clearAlso() {
	_magics.fillRect(Common::Rect(0, 0, 640, 200), 0);
	_magics.frameRect(Common::Rect(0, 45, 640, 161), 15);
}

void GraphicManager::clearTextBar() {
	_surface.fillRect(Common::Rect(24, 161, 640, 169), kColorBlack); // Black out the line of the text.
}

void GraphicManager::setAlsoLine(int x1, int y1, int x2, int y2, Color color) {
	_magics.drawLine(x1, y1, x2, y2, color);
}

void GraphicManager::drawScreenLine(int16 x, int16 y, int16 x2, int16 y2, Color color) {
	_surface.drawLine(x, y, x2, y2, color);
}

byte GraphicManager::getAlsoColor(int x1, int y1, int x2, int y2) {
	byte returnColor = 0;
	for (int16 i = x1; i <= x2; i++) {
		for (int16 j = y1; j <= y2; j++) {
			byte actColor = *(byte *)_magics.getBasePtr(i, j);
			returnColor = MAX(returnColor, actColor);
		}
	}

	return returnColor;
}

byte GraphicManager::getScreenColor(Common::Point pos) {
	return *(byte *)_surface.getBasePtr(pos.x, pos.y / 2);
}

void GraphicManager::drawSprite(AnimationType *sprite, byte picnum, int16 x, int16 y) {
	// First we make the pixels of the sprite blank.
	for (int j = 0; j < sprite->_yLength; j++) {
		for (int i = 0; i < sprite->_xLength; i++) {
			if ((x + i < _surface.w) && (y + j < _surface.h)) {
				if (((*sprite->_sil[picnum])[j][i / 8] >> ((7 - i % 8)) & 1) == 0)
					*(byte *)_surface.getBasePtr(x + i, y + j) = 0;
			}
		}
	}

	// Then we draw the picture to the blank places.
	uint16 maniPos = 0; // Because the original manitype starts at 5!!! See Graphics.h for definition.

	for (int j = 0; j < sprite->_yLength; j++) {
		for (int8 plane = 3; plane >= 0; plane--) { // The planes are in the opposite way.
			for (uint16 i = 0; i < sprite->_xLength; i += 8) {
				byte pixel = (*sprite->_mani[picnum])[maniPos++];
				for (int bit = 0; bit < 8; bit++) {
					if ((x + i + 7 < _surface.w) && (y + j < _surface.h)) {
						byte pixelBit = (pixel >> bit) & 1;
						*(byte *)_surface.getBasePtr(x + i + 7 - bit, y + j) += (pixelBit << plane);
					}
				}
			}
		}
	}
}

void GraphicManager::drawPicture(Graphics::Surface &target, const Graphics::Surface picture, uint16 destX, uint16 destY) {
	// Copy the picture to the given place on the screen.
	uint16 maxX = picture.w;
	uint16 maxY = picture.h;

	if (destX + maxX > target.w)
		maxX = target.w - destX;

	if (destY + maxY > target.h)
		maxY = target.h - destY;

	for (uint16 y = 0; y < maxY; y++) {
		for (uint16 x = 0; x < maxX; x++)
			*(byte *)target.getBasePtr(x + destX, y + destY) = *(const byte *)picture.getBasePtr(x, y);
	}
}

void GraphicManager::drawCursor(byte pos) {
	int pixPos = 24 + (pos * 8);
	// Draw the '_' character.
	for (int i = 0; i < 8; i++)
		*(byte *)_surface.getBasePtr(pixPos + i, 168) = kColorWhite;
}

void GraphicManager::drawReadyLight(Color color) {
	_surface.fillRect(Common::Rect(419, 195, 438, 197), color);
}

/**
 * This is for drawing a big "about" or "gameover" picture loaded from a file into an empty scroll.
 */
void GraphicManager::drawSign(Common::String fn, int16 xl, int16 yl, int16 y) {
	Common::File file;
	Common::String filename = Common::String::format("%s.avd", fn.c_str());

	if (!file.open(filename))
		error("AVALANCHE: Scrolls: File not found: %s", filename.c_str());

	Graphics::Surface sign; // We make a Surface object for the picture itself.
	sign = loadPictureSign(file, xl, yl);
	uint16 width = xl * 8;
	drawPicture(_scrolls, sign, kScreenWidth / 2 - width / 2, y); // x coord: center the picture.

	file.close();
}

/**
 * Draws an icon to the current scroll.
 * @remarks	Originally called 'geticon'
 */
void GraphicManager::drawIcon(int16 x, int16 y, byte which) {
	Common::File file;

	if (!file.open("icons.avd"))
		error("AVALANCHE: Scrolls: File not found: icons.avd");

	which--;
	file.seek(which * 426);

	Graphics::Surface icon = loadPictureGraphic(file);
	drawPicture(_scrolls, icon, x, y);

	icon.free();
	file.close();
}

void GraphicManager::prepareBubble(int xc, int xw, int my, Common::Point points[3]) {
	// Backup the screen before drawing the bubble.
	_scrolls.copyFrom(_surface);

	int16 talkX = _vm->_dialogs->getTalkPosX();
	// The body of the bubble.
	_scrolls.fillRect(Common::Rect(xc + talkX - xw + 9, 7, talkX + xw - 8 + xc, my + 1), _talkBackgroundColor);
	_scrolls.fillRect(Common::Rect(xc + talkX - xw - 1, 12, talkX + xw + xc + 2, my - 4), _talkBackgroundColor);

	// Top the 4 rounded corners of the bubble.
	drawPieSlice(xc + talkX + xw - 10, 11, 0, 90, 9, _talkBackgroundColor);
	drawPieSlice(xc + talkX + xw - 10, my - 4, 270, 360, 9, _talkBackgroundColor);
	drawPieSlice(xc + talkX - xw + 10, 11, 90, 180, 9, _talkBackgroundColor);
	drawPieSlice(xc + talkX - xw + 10, my - 4, 180, 270, 9, _talkBackgroundColor);

	// "Tail" of the speech bubble.
	drawTriangle(points, _talkBackgroundColor);
}

/**
 * Set the background of the text to the desired color.
 */
void GraphicManager::wipeChar(int x, int y, Color color) {
	for (int k = 0; k < 8; k++)
		*(byte *)_surface.getBasePtr(x + k, y) = color;
}

void GraphicManager::drawChar(byte ander, int x, int y, Color color) {
	byte pixel = ander;
	for (int bit = 0; bit < 8; bit++) {
		byte pixelBit = (pixel >> bit) & 1;
		if (pixelBit)
			*(byte *)_surface.getBasePtr(x + 7 - bit, y) = color;
	}
}
void GraphicManager::refreshScreen() {
	// These cycles are for doubling the screen height.
	for (uint16 y = 0; y < _screen.h / 2; y++) {
		memcpy(_screen.getBasePtr(0, y * 2), _surface.getBasePtr(0, y), _screen.w);
		memcpy(_screen.getBasePtr(0, y * 2 + 1), _surface.getBasePtr(0, y), _screen.w);
	}
	// Now we copy the stretched picture to the screen.
	g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, kScreenWidth, kScreenHeight * 2);
	g_system->updateScreen();
}

void GraphicManager::loadBackground(Common::File &file) {
	_background.free();
	_background = loadPictureRaw(file, kBackgroundWidth, kBackgroundHeight);
}

void GraphicManager::refreshBackground() {
	drawPicture(_surface, _background, 0, 10);
}

/**
 * Only used when entering the map.
 * @remarks	Originally called 'zoomout'
 */
void GraphicManager::zoomOut(int16 x, int16 y) {
	//setlinestyle(dottedln, 0, 1); TODO: Implement it with a dotted line style!!!

	saveScreen();
	for (byte i = 1; i <= 20; i ++) {
		int16 x1 = x - (x / 20) * i;
		int16 y1 = y - ((y - 10) / 20) * i;
		int16 x2 = x + (((639 - x) / 20) * i);
		int16 y2 = y + (((161 - y) / 20) * i);

		_surface.frameRect(Common::Rect(x1, y1, x2, y2), kColorWhite);
		refreshScreen();
		_vm->_system->delayMillis(17);

		restoreScreen();
	}
	removeBackup();
}

void GraphicManager::showScroll() {
	_surface.copyFrom(_scrolls); // TODO: Rework it using getSubArea !!!!!!!
}

void GraphicManager::getNaturalPicture(SpriteType &sprite) {
	sprite._type = kNaturalImage; // We simply read from the screen and later, in drawSprite() we draw it right back.
	sprite._size = sprite._xl * 8 * sprite._yl + 1;
	sprite._picture.create(sprite._xl * 8, sprite._yl + 1, Graphics::PixelFormat::createFormatCLUT8());
	for (uint16 y = 0; y < sprite._yl + 1; y++) {
		for (uint16 x = 0; x < sprite._xl * 8; x++)
			*(byte *)sprite._picture.getBasePtr(x, y) = *(byte *)_vm->_graphics->_surface.getBasePtr(sprite._x * 8 + x, sprite._y + y);
	}
}

void GraphicManager::saveScreen() {
	_backup.copyFrom(_surface);
}

void GraphicManager::removeBackup() {
	_backup.free();
}

void GraphicManager::restoreScreen() {
	_surface.copyFrom(_backup);
	refreshScreen();
}

void GraphicManager::setDialogColor(Color bg, Color text) {
	_talkBackgroundColor = bg;
	_talkFontColor = text;
}

/**
* Changes the black color of the palette to the selected one.
* @remarks	Originally called 'background'
*/
void GraphicManager::setBackgroundColor(Color newColor) {
	g_system->getPaletteManager()->setPalette(_egaPalette[kEgaPaletteIndex[newColor]], kColorBlack, 1);
}

} // End of namespace Avalanche
