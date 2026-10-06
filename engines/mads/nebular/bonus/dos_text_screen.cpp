/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "mads/nebular/bonus/dos_text_screen.h"

#include "common/system.h"
#include "common/util.h"
#include "graphics/fonts/dosfont.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

namespace MADS {
namespace RexNebular {

namespace {

static const byte kVGAPalette[16 * 3] = {
	  0,   0,   0,    0,   0, 170,    0, 170,   0,    0, 170, 170,
	170,   0,   0,  170,   0, 170,  170,  85,   0,  170, 170, 170,
	 85,  85,  85,   85,  85, 255,   85, 255,  85,   85, 255, 255,
	255,  85,  85,  255,  85, 255,  255, 255,  85,  255, 255, 255
};

} // namespace

DOSTextScreen::DOSTextScreen() {
	clear();
}

void DOSTextScreen::clear(byte character, byte attribute) {
	for (int i = 0; i < kColumns * kRows; ++i) {
		_cells[i].character = character;
		_cells[i].attribute = attribute;
	}
}

bool DOSTextScreen::isValidCell(int x, int y) const {
	return x >= 0 && x < kColumns && y >= 0 && y < kRows;
}

const DOSTextScreen::Cell &DOSTextScreen::getCell(int x, int y) const {
	assert(isValidCell(x, y));
	return _cells[indexOf(x, y)];
}

void DOSTextScreen::setCell(int x, int y, byte character, byte attribute) {
	if (!isValidCell(x, y))
		return;

	Cell &cell = _cells[indexOf(x, y)];
	cell.character = character;
	cell.attribute = attribute;
}

void DOSTextScreen::setCharacter(int x, int y, byte character) {
	if (isValidCell(x, y))
		_cells[indexOf(x, y)].character = character;
}

void DOSTextScreen::setAttribute(int x, int y, byte attribute) {
	if (isValidCell(x, y))
		_cells[indexOf(x, y)].attribute = attribute;
}

void DOSTextScreen::fill(const Common::Rect &rect, byte character, byte attribute) {
	const int left = MAX<int>(0, rect.left);
	const int top = MAX<int>(0, rect.top);
	const int right = MIN<int>(kColumns, rect.right);
	const int bottom = MIN<int>(kRows, rect.bottom);

	for (int y = top; y < bottom; ++y)
		for (int x = left; x < right; ++x)
			setCell(x, y, character, attribute);
}

void DOSTextScreen::recolor(const Common::Rect &rect, byte attribute) {
	const int left = MAX<int>(0, rect.left);
	const int top = MAX<int>(0, rect.top);
	const int right = MIN<int>(kColumns, rect.right);
	const int bottom = MIN<int>(kRows, rect.bottom);

	for (int y = top; y < bottom; ++y)
		for (int x = left; x < right; ++x)
			setAttribute(x, y, attribute);
}

void DOSTextScreen::drawBox(const Common::Rect &rect, byte attribute, bool doubleLine) {
	if (rect.width() < 2 || rect.height() < 2)
		return;

	const byte horizontal = doubleLine ? 0xCD : 0xC4;
	const byte vertical = doubleLine ? 0xBA : 0xB3;
	const byte topLeft = doubleLine ? 0xC9 : 0xDA;
	const byte topRight = doubleLine ? 0xBB : 0xBF;
	const byte bottomLeft = doubleLine ? 0xC8 : 0xC0;
	const byte bottomRight = doubleLine ? 0xBC : 0xD9;
	const int right = rect.right - 1;
	const int bottom = rect.bottom - 1;

	setCell(rect.left, rect.top, topLeft, attribute);
	setCell(right, rect.top, topRight, attribute);
	setCell(rect.left, bottom, bottomLeft, attribute);
	setCell(right, bottom, bottomRight, attribute);

	for (int x = rect.left + 1; x < right; ++x) {
		setCell(x, rect.top, horizontal, attribute);
		setCell(x, bottom, horizontal, attribute);
	}
	for (int y = rect.top + 1; y < bottom; ++y) {
		setCell(rect.left, y, vertical, attribute);
		setCell(right, y, vertical, attribute);
	}
}

void DOSTextScreen::drawShadow(const Common::Rect &rect, byte shadowAttribute) {
	// MADS uses a two-column right shadow and a one-row bottom shadow by
	// recoloring the desktop cells that are already present.
	recolor(Common::Rect(rect.right, rect.top + 1,
			MIN<int>(kColumns, rect.right + 2),
			MIN<int>(kRows, rect.bottom + 1)), shadowAttribute);
	recolor(Common::Rect(MIN<int>(kColumns, rect.left + 2), rect.bottom,
			MIN<int>(kColumns, rect.right + 2),
			MIN<int>(kRows, rect.bottom + 1)), shadowAttribute);
}

void DOSTextScreen::drawSeparator(const Common::Rect &rect, int y, byte attribute) {
	if (y <= rect.top || y >= rect.bottom - 1)
		return;

	setCell(rect.left, y, 0xC7, attribute);
	setCell(rect.right - 1, y, 0xB6, attribute);
	for (int x = rect.left + 1; x < rect.right - 1; ++x)
		setCell(x, y, 0xC4, attribute);
}

Common::String DOSTextScreen::visibleText(const Common::String &source) {
	Common::String result;
	for (uint i = 0; i < source.size(); ++i) {
		if (source[i] == '~' && i + 1 < source.size())
			continue;
		result += source[i];
	}
	return result;
}

int DOSTextScreen::visibleTextWidth(const Common::String &source) {
	return (int)visibleText(source).size();
}

char DOSTextScreen::accelerator(const Common::String &source) {
	for (uint i = 0; i + 1 < source.size(); ++i)
		if (source[i] == '~')
			return source[i + 1];
	return 0;
}

int DOSTextScreen::drawText(int x, int y, const Common::String &text,
		byte attribute, byte hotkeyAttribute, bool parseAccelerator, int maxCells) {
	int used = 0;
	bool hotkey = false;

	for (uint i = 0; i < text.size(); ++i) {
		if (parseAccelerator && text[i] == '~' && i + 1 < text.size()) {
			hotkey = true;
			continue;
		}
		if (maxCells >= 0 && used >= maxCells)
			break;

		setCell(x + used, y, (byte)text[i], hotkey ? hotkeyAttribute : attribute);
		hotkey = false;
		++used;
	}

	return used;
}

int DOSTextScreen::drawCenteredText(int y, int left, int right,
		const Common::String &text, byte attribute, byte hotkeyAttribute,
		bool parseAccelerator) {
	const int width = parseAccelerator ? visibleTextWidth(text) : (int)text.size();
	const int x = left + MAX<int>(0, (right - left - width) / 2);
	return drawText(x, y, text, attribute, hotkeyAttribute, parseAccelerator,
			MAX<int>(0, right - x));
}

void DOSTextScreen::drawBorderText(const Common::Rect &rect, int y,
		const Common::String &text, byte attribute, byte hotkeyAttribute) {
	const int width = visibleTextWidth(text) + 2;
	const int x = rect.left + MAX<int>(1, (rect.width() - width) / 2);

	for (int i = 0; i < width && x + i < rect.right - 1; ++i)
		setCell(x + i, y, 0x20, attribute);
	drawText(x + 1, y, text, attribute, hotkeyAttribute, true,
			MAX<int>(0, rect.right - x - 2));
}

void DOSTextScreen::drawTitle(const Common::Rect &rect, const Common::String &title,
		byte attribute, byte hotkeyAttribute) {
	drawBorderText(rect, rect.top, title, attribute, hotkeyAttribute);
}

void DOSTextScreen::drawFooter(const Common::Rect &rect, const Common::String &text,
		byte attribute, byte hotkeyAttribute) {
	drawBorderText(rect, rect.bottom - 1, text, attribute, hotkeyAttribute);
}

bool DOSTextScreen::render(Graphics::Surface &surface, bool blinkVisible) const {
	if (surface.w != kRasterWidth || surface.h != kRasterHeight ||
			surface.format.bytesPerPixel != 1)
		return false;

	Graphics::DosFont font;
	Graphics::Surface glyphSurface;
	glyphSurface.create(kCellWidth, 8, surface.format);

	for (int cellY = 0; cellY < kRows; ++cellY) {
		for (int cellX = 0; cellX < kColumns; ++cellX) {
			const Cell &cell = _cells[indexOf(cellX, cellY)];
			const byte foreground = cell.attribute & 0x0F;
			const byte background = (cell.attribute >> 4) & 0x07;
			const bool drawForeground = !(cell.attribute & 0x80) || blinkVisible;
			const int pixelX = cellX * kCellWidth;
			const int pixelY = cellY * kCellHeight;

			surface.fillRect(Common::Rect(pixelX, pixelY,
					pixelX + kCellWidth, pixelY + kCellHeight), background);
			if (!drawForeground || cell.character == 0x20 || cell.character == 0)
				continue;

			glyphSurface.fillRect(Common::Rect(0, 0, kCellWidth, 8), 0);
			font.drawChar(&glyphSurface, cell.character, 0, 0, 1);
			// The native 80-column text mode uses 8x16 cells. ScummVM's DOS
			// font contains the complete 8x8 CP437 set, including borders and
			// shading, so expand each scanline exactly twice.
			for (int destY = 0; destY < kCellHeight; ++destY) {
				const int sourceY = destY / 2;
				for (int destX = 0; destX < kCellWidth; ++destX) {
					if (*((const byte *)glyphSurface.getBasePtr(destX, sourceY)) != 0)
						*((byte *)surface.getBasePtr(pixelX + destX,
								pixelY + destY)) = foreground;
				}
			}
		}
	}

	glyphSurface.free();
	return true;
}

void DOSTextScreen::installVGAPalette() {
	if (g_system && g_system->getPaletteManager())
		g_system->getPaletteManager()->setPalette(kVGAPalette, 0, 16);
}

} // namespace RexNebular
} // namespace MADS
