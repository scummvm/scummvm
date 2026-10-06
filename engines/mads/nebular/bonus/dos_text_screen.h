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

#ifndef MADS_NEBULAR_BONUS_DOS_TEXT_SCREEN_H
#define MADS_NEBULAR_BONUS_DOS_TEXT_SCREEN_H

#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Graphics {
struct Surface;
}

namespace MADS {
namespace RexNebular {

/** Semantic 80x25 DOS text screen rendered as 8x16 cells. */
class DOSTextScreen {
public:
	enum {
		kColumns = 80,
		kRows = 25,
		kCellWidth = 8,
		kCellHeight = 16,
		kRasterWidth = kColumns * kCellWidth,
		kRasterHeight = kRows * kCellHeight
	};

	struct Cell {
		byte character;
		byte attribute;
	};

	DOSTextScreen();

	void clear(byte character = 0x20, byte attribute = 0x07);
	bool isValidCell(int x, int y) const;
	const Cell &getCell(int x, int y) const;
	void setCell(int x, int y, byte character, byte attribute);
	void setCharacter(int x, int y, byte character);
	void setAttribute(int x, int y, byte attribute);

	void fill(const Common::Rect &rect, byte character, byte attribute);
	void recolor(const Common::Rect &rect, byte attribute);
	void drawBox(const Common::Rect &rect, byte attribute, bool doubleLine = true);
	void drawShadow(const Common::Rect &rect, byte shadowAttribute = 0x08);
	void drawSeparator(const Common::Rect &rect, int y, byte attribute);
	void drawTitle(const Common::Rect &rect, const Common::String &title,
			byte attribute, byte hotkeyAttribute = 0x0F);
	void drawFooter(const Common::Rect &rect, const Common::String &text,
			byte attribute, byte hotkeyAttribute = 0x0F);

	/**
	 * Draw CP437/ASCII text. A tilde marks the following accelerator and
	 * is not displayed. Returns the number of cells consumed.
	 */
	int drawText(int x, int y, const Common::String &text, byte attribute,
			byte hotkeyAttribute, bool parseAccelerator = true, int maxCells = -1);
	int drawCenteredText(int y, int left, int right, const Common::String &text,
			byte attribute, byte hotkeyAttribute, bool parseAccelerator = true);

	/** Render to an existing 640x400 CLUT8 surface. */
	bool render(Graphics::Surface &surface, bool blinkVisible = true) const;

	static void installVGAPalette();
	static Common::String visibleText(const Common::String &source);
	static int visibleTextWidth(const Common::String &source);
	static char accelerator(const Common::String &source);

private:
	Cell _cells[kColumns * kRows];

	static int indexOf(int x, int y) { return y * kColumns + x; }
	void drawBorderText(const Common::Rect &rect, int y,
			const Common::String &text, byte attribute, byte hotkeyAttribute);
};

} // namespace RexNebular
} // namespace MADS

#endif // MADS_NEBULAR_BONUS_DOS_TEXT_SCREEN_H
