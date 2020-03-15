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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef DRAGONS_FONT_H
#define DRAGONS_FONT_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "graphics/surface.h"
#include "common/rect.h"

namespace Dragons {

class DragonsEngine;
class BigfileArchive;
class Screen;

class Font {
private:
	uint32 _size;
	uint16 *_map;
	byte *_pixels;
	uint32 _numChars;

public:
	Font(Common::SeekableReadStream &stream, uint32 mapSize, uint32 pixelOffset, uint32 pixelSize);
	~Font();
	Graphics::Surface *render(uint16 *text, uint16 length);
	void renderToSurface(Graphics::Surface *surface, int16 x, int16 y, uint16 *text, uint16 length);

private:
	uint16 mapChar(uint16 in);
};

class FontManager {
public:
	Font *_fonts[3];

private:
	uint16 _dat_80086f48_fontColor_flag;
	DragonsEngine *_vm;
	Screen *_screen;
	Graphics::Surface *_surface;
	byte *_boxFontChr;

	int _numTextEntries;

public:
	FontManager(DragonsEngine *vm, Screen *screen, BigfileArchive *bigfileArchive);
	~FontManager();
	void addText(int16 x, int16 y, uint16 *text, uint16 length, uint8 fontType);
	void addAsciiText(int16 x, int16 y, const char *text, uint16 length, uint8 fontType);
	void draw();
	void clearText();
	void updatePalette();
	void drawTextDialogBox(uint32 x1, uint32 y1, uint32 x2, uint32 y2);
	void clearTextDialog(uint32 x1, uint32 y1, uint32 x2, uint32 y2);

private:
	Font *loadFont(uint16 index, Common::SeekableReadStream &stream);
	void drawBoxChar(uint32 x, uint32 y, uint8 tileIndex);
};

} // End of namespace Dragons

#endif //DRAGONS_FONT_H
