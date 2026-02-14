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
	int32 x = 0;				// X position relative	to GrBuff(0, 0)
	int32 y = 0;				// Y position relative	to GrBuff(0, 0)
	int32 scale_x = 0;			// X scale factor (can be negative for reverse draw)
	int32 scale_y = 0;			// Y scale factor (can't be negative)
	uint8 *depth_map = nullptr;	// Depth code array for destination (doesn't care if srcDepth is 0)
	RGBcolor *Pal = nullptr;	// Palette for shadow draw (doesn't care if SHADOW bit is not set in Src.encoding)
	uint8 *ICT = nullptr;		// Inverse Color Table (doesn't care if SHADOW bit is not set in Src.encoding)
	uint8 depth = 0;			// Depth code for source (0 if no depth processing)
};

struct RendCell {
	uint32 Pack = 0;
	uint32 Stream = 0;
	long   hot_x = 0;
	long   hot_y = 0;
	uint32 Width = 0;
	uint32 Height = 0;
	uint32 Comp = 0;
	uint32 Reserved[8] = {};
	uint8 *data = nullptr;
};

enum {
	kEndOfLine = 0,
	kEndOfSprite = 1,
	kJumpXY = 2
};

typedef uint32 RenderResult;

typedef RenderResult(*RenderFunc)();

struct Rend_Globals {
	uint8 *_sourceAddress = nullptr;
	uint8 *_destinationAddress = nullptr;
	uint8 *_depthAddress = nullptr;
	uint8 _spriteDepth = 0;
	uint8 *_InverseColorTable = nullptr;
	int32 _X_scale = 0;
	int32 _LeftPorch = 0;
	int32 _RightPorch = 0;
	int32 _StartingPixelPos = 0;
	int32 _X_error = 0;
	int _Increment = 0;
	RGBcolor *_Palette = nullptr;
};

void GetUpdateRectangle(int32 x, int32 y, int32 hot_x, int32 hot_y, int32 scale_x, int32 scale_y, int32 Width, int32 Height, M4Rect *UpdateRect);
void render_sprite_to_8BBM(RendGrBuff *Destination, DrawRequestX *dr, RendCell *Frame, M4Rect *ClipRectangle, M4Rect *UpdateRect);

} // namespace M4

#endif
