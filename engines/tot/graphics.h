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

#include "common/system.h"

#include "graphics/fonts/bgifont.h"

namespace Tot {

const int kTextAreaSize = 320 * 70 + 4;

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
	int euroTextWidth(const Common::String &str);
	void biosText(const Common::String &str, int x, int y, uint32 color);

	void clear();

	void clearActionLine();
	void writeActionLine(const Common::String &str);

	void drawFullScreen(byte *screen);
	void copyFromScreen(byte *&screen);
	void drawScreen(byte *screen, bool offsetSize = true);
	void restoreBackground();
	void restoreBackgroundArea(uint x, uint y, uint x2, uint y2);

	void sceneTransition(bool fadeToBlack, byte *screen, byte effectNumber);
	void sceneTransition(bool fadeToBlack, byte *screen);

	void advancePaletteAnim();
	void printColor(int x, int y, int color);
	void updateSceneArea(int speed = 1);

	/**
	 * Aux for palette animation
	 */
	byte _paletteAnimFrame = 0;
	/**
	 * Delay of palette animation
	 */
	byte _palAnimStep = 0;
	/**
	 * 54 color palette slice.
	 */
	byte _palAnimSlice[768] = { 0 };
	/**
	 * General palette
	 */
	byte _pal[768] = { 0 };

	/**
	 * TextArea for animations text
	 */
	byte *_textAreaBackground = (byte *)malloc(kTextAreaSize);
private:
	signed char fadeData[256][256];
	Graphics::BgiFont *_litt;
	Graphics::BgiFont *_euro;
	Graphics::Font *_dosFont;
};

} // End of namespace Tot
#endif
