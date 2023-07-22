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

#ifndef M4_PLATFORM_DRAW_H
#define M4_PLATFORM_DRAW_H

#include "m4/m4_types.h"

namespace M4 {

/**
 * Decode an 8BPP RLE encoded image
 */
extern void RLE8Decode(const uint8 *inBuff, uint8 *outBuff, uint32 pitch);

/**
 * Given RLE data, skips over a given number of encoded lines
 */
extern uint8 *SkipRLE_Lines(uint32 linesToSkip, uint8 *rleData);

extern void RLE_Draw(Buffer *src, Buffer *dest, int32 x, int32 y);
extern void RLE_DrawRev(Buffer *src, Buffer *dest, int32 x, int32 y);
extern void RLE_DrawDepth(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode);
extern void RLE_DrawDepthRev(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode);

extern void Raw_Draw(Buffer *src, Buffer *dest, int32 x, int32 y);
extern void Raw_DrawRev(Buffer *src, Buffer *dest, int32 x, int32 y);
extern void Raw_DrawDepth(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode);
extern void Raw_DrawDepthRev(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode);

extern void Raw_SDraw(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 *palette, uint8 *inverseColorTable);
extern void Raw_SDrawRev(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 *palette, uint8 *inverseColorTable);
extern void Raw_SDrawDepth(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, uint8 *palette, uint8 *inverseColorTable);
extern void Raw_SDrawDepthRev(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, uint8 *palette, uint8 *inverseColorTable);

extern void RLE_DrawOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset);
extern void RLE_DrawDepthOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset);
extern void RLE_DrawRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset);
extern void RLE_DrawDepthRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset);

extern void Raw_DrawOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset);
extern void Raw_DrawRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset);
extern void Raw_DrawDepthOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset);
extern void Raw_DrawDepthRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset);

extern void Raw_SDrawOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset, uint8 *palette, uint8 *inverseColorTable);
extern void Raw_SDrawRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset, uint8 *palette, uint8 *inverseColorTable);
extern void Raw_SDrawDepthOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset, uint8 *palette, uint8 *inverseColorTable);
extern void Raw_SDrawDepthRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset, uint8 *palette, uint8 *inverseColorTable);

} // namespace M4

#endif
