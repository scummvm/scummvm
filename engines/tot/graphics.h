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

#include "playanim.h"
#include "util.h"

namespace Tot {


void initGraph();
void getVirtualImg(uint getcoordx1, uint getcoordy1, uint getcoordx2, uint getcoordy2,
				   byte *backgroundScreen, byte *image);
void putVirtualImg(uint putcoordx, uint putcoordy, byte *backgroundScreen, byte *image);
void clear();
void drawFullScreen(byte *screen);
void copyFromScreen(byte *&screen);
void drawScreen(byte *screen, boolean offsetSize = true);
void putImg(uint coordx, uint coordy, byte *image, boolean transparency = false);
void putShape(uint coordx, uint coordy, byte *image);
void getImg(uint coordx1, uint coordy1, uint coordx2, uint coordy2, byte *image);
void loadPalette(Common::String image);
void effect(byte effectNumber, boolean fadeToBlack, byte *screen);
void turnLightOn();
void totalFadeOut(byte red);
void partialFadeOut(byte numCol);
void totalFadeIn(uint paletteNumber, Common::String paletteName);
void redFadeIn(palette palette);
void partialFadeIn(byte numCol);
void updatePalette(byte paletteIndex);
void changePalette(palette fromPalette, palette toPalette);
void changeRGBBlock(byte initialColor, uint numColors, void *rgb);
void processingActive();
void rectangle(int x1, int y1, int x2, int y2, byte color);
void rectangle(uint x1, uint y1, uint x2, uint y2, byte color);
void handPantallaToFondo();
void copyPalette(palette from, palette to);
void loadAnimationIntoBuffer(Common::SeekableReadStream *stream, byte *&buf, int sizeanimado);
} // End of namespace Tot

#endif
