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
#ifndef TOT_GRAPHICS_H
#define TOT_GRAPHICS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "graphics/font.h"

#include "tot/font/bgifont.h"
// #include "tot/font/biosfont.h"
// #include "tot/vars.h"

namespace Tot {

class GraphicsManager {
public:
	GraphicsManager();
	~GraphicsManager();

	void init();
	void fixPalette(byte *palette, uint num = 768);
	byte *getPalette();
	void setPalette(byte palette[768], uint start = 0, uint num = 256);
	void loadPaletteFromFile(Common::String image);
	void turnLightOn();
	void totalFadeOut(byte red);
	void partialFadeOut(byte numCol);
	void totalFadeIn(uint paletteNumber, Common::String paletteName);
	void redFadeIn(byte *palette);
	void partialFadeIn(byte numCol);
	void updatePalette(byte paletteIndex);
	void fadePalettes(byte *fromPalette, byte *toPalette);
	void copyPalette(byte *from, byte *to);

	void putImg(uint coordx, uint coordy, byte *image, bool transparency = false);
	void getImg(uint coordx1, uint coordy1, uint coordx2, uint coordy2, byte *image);
	void putShape(uint coordx, uint coordy, byte *image);
	void putImageArea(uint putcoordx, uint putcoordy, byte *backgroundScreen, byte *image);
	void getImageArea(uint getcoordx1, uint getcoordy1, uint getcoordx2, uint getcoordy2, byte *backgroundScreen, byte *image);

	void littText(const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align = Graphics::kTextAlignLeft, bool alignCenterY = false);
	void euroText(const Common::String &str, int x, int y, uint32 color, Graphics::TextAlign align = Graphics::kTextAlignLeft, bool alignCenterY = false);
	void biosText(const Common::String &str, int x, int y, uint32 color);

	void clear();

	void clearActionLine();
	void writeActionLine(const Common::String &str);

	void drawFullScreen(byte *screen);
	void copyFromScreen(byte *&screen);
	void drawScreen(byte *screen, bool offsetSize = true);
	void restoreBackground();
	void restoreBackgroundArea(uint x, uint y, uint x2, uint y2);

	void sceneTransition(byte effectNumber, bool fadeToBlack, byte *screen);

	void loadAnimationIntoBuffer(Common::SeekableReadStream *stream, byte *&buf, int animSize);
	void printColor(int x, int y, int color);


private:
	void updateSceneAreaIfNeeded(int speed = 1);
	signed char fadeData[256][256];
	Graphics::BgiFont _litt;
	Graphics::BgiFont _euro;
	Graphics::Font *_bios;
};

} // End of namespace Tot
#endif
