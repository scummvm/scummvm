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

#ifndef M4_GRAPHICS_GR_SPRITE_H
#define M4_GRAPHICS_GR_SPRITE_H

#include "m4/m4_types.h"
#include "m4/gui/gui.h"

namespace M4 {

enum {
	NO_COMPRESS     = 0x00,
	RLE8            = 0x01,
	SHADOW			= 0x80
};

struct DrawRequest {
	Buffer *Src = nullptr;		// sprite source buffer
	Buffer *Dest = nullptr;		// destination buffer
	int32 x = 0;			// x position relative	to Destination(0, 0)
	int32 y = 0;			// y position relative	to Destination(0, 0)
	int32 scaleX = 0;		// x scale factor (can be negative for reverse draw)
	int32 scaleY = 0;		// y scale factor (can't be negative)
	uint8 *depthCode = nullptr;	// depth code array for	destination (doesn't care if srcDepth is 0)
	uint8 *Pal = nullptr;			// palette for shadow draw (doesn't care if SHADOW bit is not set in Src.encoding)
	uint8 *ICT = nullptr;		// Inverse Color Table (doesn't care if SHADOW bit is not set in Src.encoding)
	uint8 srcDepth = 0;		// depth code for source (0 if no depth processing)
};


uint32 gr_sprite_RLE8_encode(Buffer *Source, Buffer *Dest);
uint8 gr_sprite_draw(DrawRequest *DrawReq);
void gr_sprite_draw_scaled(M4sprite *srcSprite, Buffer *destBuf, int32 destX, int32 destY, int32 scale);

} // namespace M4

#endif
