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
#ifndef TOT_UTIL_H
#define TOT_UTIL_H

#include "tot/tot.h"

namespace Tot {

extern const int kEnforcedTextAnimDelay;

void showError(int code);

// Delays a specified number of milliseconds.
void delay(uint16 ms);

int engine_start();

void setRGBPalette(int color, int r, int g, int b);

void littText(int x, int y, const Common::String &text, byte color, Graphics::TextAlign align = Graphics::kTextAlignStart);
void littText(int x, int y, const char *text, byte color, Graphics::TextAlign align = Graphics::kTextAlignStart);

void euroText(int x, int y, const Common::String &text, byte color, Graphics::TextAlign align = Graphics::kTextAlignStart);
void euroText(int x, int y, const char *text, byte color, Graphics::TextAlign align = Graphics::kTextAlignStart);

void biosText(int x, int y, const Common::String &text, byte color);

uint imagesize(int x, int y, int x2, int y2);

void rectangle(int x1, int y1, int x2, int y2, byte color);

void line(uint x1, uint y1, uint x2, uint y2, byte color);

void bar(int x1, int y1, int x2, int y2, byte color);

void putpixel(int x, int y, byte color);

void emptyLoop();

void emptyLoop2();

void waitForKey();

void changeGameSpeed(Common::Event e);

inline bool odd(int32 i) { return i % 2 != 0; }

int getRandom(int range);

Common::String getObjectName(int idx);

Common::KeyCode hotKeyFor(HOTKEYS hotkey);

Common::String getActionLineText(int idx);

const char *const *getFullScreenMessagesByCurrentLanguage();

const char *const *getAnimMessagesByCurrentLanguage();

const char *const *getHardcodedTextsByCurrentLanguage();

const int32 *getOffsetsByCurrentLanguage();

void newSecondaryAnimationFrame();

inline bool isLanguageSpanish() {
	return g_engine->_lang == Common::ES_ESP;
};

} // End of namespace Tot

#endif
