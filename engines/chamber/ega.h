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

#ifndef CHAMBER_EGA_H
#define CHAMBER_EGA_H

#include "graphics/surface.h"

namespace Chamber {

// EGA screen dimensions (same as CGA)
#define EGA_WIDTH  320
#define EGA_HEIGHT 200

// EGA uses 1 byte per pixel (CLUT8 linear, no interlacing)
#define EGA_BYTES_PER_LINE EGA_WIDTH
#define EGA_SCREEN_SIZE    (EGA_WIDTH * EGA_HEIGHT)

// Planar background: 4 planes × 8000 bytes
#define EGA_PLANAR_SIZE    (EGA_WIDTH / 8 * EGA_HEIGHT)   // 8000 bytes per plane

extern byte *ega_backbuffer; // aliases backbuffer

// CGA palette 1 high-intensity → EGA color index mapping
extern const byte cga_to_ega_color[4];

// --- resource loader ---
Graphics::Surface *ega_loadFond(const char *filename);

// --- background restore ---
// Saved clean copy of decoded FOND.EGA for room background restoration
extern byte ega_fond_clean[EGA_SCREEN_SIZE];
void ega_drawBackground(byte *target);

} // End of namespace Chamber

#endif // CHAMBER_EGA_H
