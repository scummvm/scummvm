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

#ifndef SCUMM_SMUSH_REBEL_FONT_REBEL2_H
#define SCUMM_SMUSH_REBEL_FONT_REBEL2_H

#include "common/scummsys.h"
#include "common/rect.h"

#include "scumm/charset_v7.h"

namespace Scumm {

class ScummEngine;
class NutRenderer;

struct Rebel2FontSet {
	enum {
		kMaxFonts = 5
	};

	NutRenderer *fonts[kMaxFonts];
	int numFonts;
	int defaultFont;

	Rebel2FontSet();
	NutRenderer *getFont(int id) const;
};

NutRenderer *makeRebel2Font(ScummEngine *vm, const char *filename);
NutRenderer *makeRebel2SpriteFromData(ScummEngine *vm, const byte *data, int32 dataSize);
int drawRebel2Char(NutRenderer *font, byte *buffer, Common::Rect &clipRect, int x, int y,
		int pitch, int16 col, byte chr);
int getRebel2StringWidth(const Rebel2FontSet &fontSet, const char *str, uint len);
int getRebel2StringHeight(const Rebel2FontSet &fontSet, const char *str, uint len);
void drawRebel2String(const Rebel2FontSet &fontSet, const char *str, uint len, byte *buffer,
		Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags);
void drawRebel2StringWrap(const Rebel2FontSet &fontSet, const char *str, uint len, byte *buffer,
		Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags);

} // End of namespace Scumm

#endif
