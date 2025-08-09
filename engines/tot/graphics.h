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

#include "engines/tot/vars.h"
#include "engines/tot/util.h"

namespace Tot {

void initGraph();
void drawFullScreen(byte *screen);
void copyFromScreen(byte *&screen);
void drawScreen(byte *screen, bool offsetSize = true);

void loadPalette(Common::String image);
void screenTransition(byte effectNumber, bool fadeToBlack, byte *screen);
void turnLightOn();
void totalFadeOut(byte red);
void partialFadeOut(byte numCol);
void totalFadeIn(uint paletteNumber, Common::String paletteName);
void redFadeIn(palette palette);
void partialFadeIn(byte numCol);
void updatePalette(byte paletteIndex);
void changePalette(palette fromPalette, palette toPalette);
void changeRGBBlock(byte initialColor, uint numColors, byte *rgb);
void restoreBackground();
void copyPalette(palette from, palette to);
void loadAnimationIntoBuffer(Common::SeekableReadStream *stream, byte *&buf, int animSize);
} // End of namespace Tot

#endif
