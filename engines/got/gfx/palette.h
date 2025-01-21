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

#ifndef GOT_GFX_PALETTE_H
#define GOT_GFX_PALETTE_H

#include "graphics/paletteman.h"

namespace Got {
namespace Gfx {

struct Palette63 {
	byte _pal[PALETTE_SIZE] = {};
	Palette63() {}
	Palette63(const byte *pal);

	operator const byte *() const {
		return _pal;
	}
};

extern void loadPalette();
extern void setScreenPal();
extern void setPalette(const byte *pal);
extern void xSetPal(byte color, byte R, byte G, byte B);
extern void xSetPal(const byte *pal);
extern void xGetPal(byte *pal, int numColors, int startIndex);
extern void fadeOut();
extern void fadeIn(const byte *pal = nullptr);

} // namespace Gfx
} // namespace Got

#endif
