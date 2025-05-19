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

	void fixPalette(byte *palette, uint num = 768);
	void getPalette(byte *palette);
	byte *getPalette();
	void setPalette(byte palette[768], uint num = 256);

	void restoreBackground(uint x, uint y, uint x2, uint y2);
	void littText(const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align = Graphics::kTextAlignLeft, bool alignCenterY = false);
	void euroText(const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align = Graphics::kTextAlignLeft, bool alignCenterY = false);
	void biosText(const Common::String &str, int x, int y, uint32 color);
	void clear();
	void clearActionLine();
	void writeActionLine(const Common::String &str);
	void printColor(int x, int y, int color);
	Graphics::BgiFont _litt;
	Graphics::BgiFont _euro;
	Graphics::Font *_bios;

private:
	signed char datosfundido[256][256];

	const int FONT_LITT_OFFSET_ES = 153584;
	const int FONT_EURO_OFFSET_ES = 159681;
	const int FONT_LITT_OFFSET_EN = 153010;
	const int FONT_EURO_OFFSET_EN = 159107;
};

} // End of namespace Tot
#endif
