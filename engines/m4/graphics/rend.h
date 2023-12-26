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

#ifndef M4_GRAPHICS_REND_H
#define M4_GRAPHICS_REND_H

#include "m4/m4_types.h"
#include "m4/gui/gui.h"

namespace M4 {

struct RGBcolor {
	uint8 b, g, r;
};

struct RendGrBuff {
	uint32 Width = 0;
	uint32 Height = 0;
	void *PixMap = nullptr;
	byte encoding = 0;
	uint32 Pitch = 0;
};

struct DrawRequestX {
	int32 x;			// X position relative	to GrBuff(0, 0)
	int32 y;			// Y position relative	to GrBuff(0, 0)
	int32 scale_x;		// X scale factor (can be negative for reverse draw)
	int32 scale_y;		// Y scale factor (can't be negative)
	uint8 *depth_map;	// Depth code array for destination (doesn't care if srcDepth is 0)
	RGBcolor *Pal;		// Palette for shadow draw (doesn't care if SHADOW bit is not set in Src.encoding)
	uint8 *ICT;			// Inverse Color Table (doesn't care if SHADOW bit is not set in Src.encoding)
	uint8 depth;		// Depth code for source (0 if no depth processing)
};

struct RendCell {
	uint32 Pack;
	uint32 Stream;
	long   hot_x;
	long   hot_y;
	uint32 Width;
	uint32 Height;
	uint32 Comp;
	uint32 Reserved[8];
	uint8 *data;
};

enum {
	kEndOfLine = 0,
	kEndOfSprite = 1,
	kJumpXY = 2
};

typedef uint32 RenderResult;

typedef RenderResult(*RenderFunc)();

struct Rend_Globals {
	uint8 *_sourceAddress, *_destinationAddress, *_depthAddress, _spriteDepth, *_InverseColorTable;
	int32 _X_scale, _LeftPorch, _RightPorch, _StartingPixelPos, _X_error;
	int _Increment;
	RGBcolor *_Palette;
};

void GetUpdateRectangle(int32 x, int32 y, int32 hot_x, int32 hot_y, int32 scale_x, int32 scale_y, int32 Width, int32 Height, M4Rect *UpdateRect);
void render_sprite_to_8BBM(RendGrBuff *Destination, DrawRequestX *dr, RendCell *Frame, M4Rect *ClipRectangle, M4Rect *UpdateRect);

} // namespace M4

#endif
