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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef TOT_GRAPHMAN_H
#define TOT_GRAPHMAN_H

#include "common/scummsys.h"
#include "common/system.h"
#include "graphics/font.h"

#include "tot/font/bgifont.h"
#include "tot/font/biosfont.h"

namespace Tot {

class GraphicsManager {
public:
	GraphicsManager();
	~GraphicsManager();

	void printPalette();

	void fixPalette(byte palette[768], uint num = 768);
	void getPalette(byte *palette);
	byte *getPalette();
	void setPalette(byte palette[768], uint num = 256);
	void changePalette(byte origin[768], byte target[768]);
	byte *loadPalette(Common::String fileName);

	void fadeOut(byte rojo = 0);
	void fadeIn(uint numeropaleta, Common::String nombrepaleta);

	void littText(const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign  align = Graphics::kTextAlignLeft, bool alignCenterY = false);
	void euroText(const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align = Graphics::kTextAlignLeft, bool alignCenterY = false);
	void biosText(const Common::String &str, int x, int y, uint32 color);
	void blit(const Graphics::Surface *src, Common::Rect bounds);
	void copyPixels(int x, int y, byte *pixels, uint size);
	void copyRect(Common::Rect rect, byte *pixels);

	void effect(int effectNumber, bool toBlack);

	void restoreBackground(uint x, uint y, uint x2, uint y2);
	void clear();
	void clearActionLine();
	void writeActionLine(const Common::String &str);
	void printColor(int x, int y, int color);
	Graphics::BgiFont _litt;
	Graphics::BgiFont _euro;
	Graphics::Font *_bios;

private:
	const int FONT_LITT_OFFSET = 153584;
	const int FONT_EURO_OFFSET = 159681;
	signed char datosfundido[256][256];
	void updateScreen();
	void updateScreen(const Common::Rect &r);
	// void restoreBackground(const Common::Rect &r);
	Graphics::Surface *_currentBuffer;
	Graphics::Surface *_backBuffer;
};

} // End of namespace Tot
#endif
