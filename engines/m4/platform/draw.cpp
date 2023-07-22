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

#include "common/algorithm.h"
#include "common/textconsole.h"
#include "m4/platform/draw.h"

namespace M4 {

#define EOL_CODE 3
#define END_CODE 2

void RLE8Decode(const uint8 *inBuff, uint8 *outBuff, uint32 pitch) {
	byte val, count;
	int line = 0, numY = 0;
	byte *destP = outBuff;

	for (;;) {
		count = *inBuff++;

		if (count != 0xff) {
			// Basic run length
			val = *inBuff++;
			Common::fill(destP, destP + count, val);
			destP += count;

		} else {
			count = *inBuff++;

			if (count > EOL_CODE) {
				// Block of uncompressed pixels to copy
				Common::copy(inBuff, inBuff + count, destP);
				inBuff += count;
				destP += count;

			} else if (count == EOL_CODE) {
				// End of Line code
				++line;
				destP = outBuff + line * pitch;

			} else if (count == END_CODE) {
				break;

			} else {
				// Move down by X, Y amount
				destP += *inBuff++;		// x amount
				numY = *inBuff++;		// y amount
				line += numY;
				destP += numY * pitch;
			}
		}
	}
}

uint8 *SkipRLE_Lines(uint32 linesToSkip, uint8 *RLE_Data) {
	error("TODO: RLE");
}

void RLE_Draw(Buffer *src, Buffer *dest, int32 x, int32 y) {
	error("TODO: RLE");
}

void RLE_DrawRev(Buffer *src, Buffer *dest, int32 x, int32 y) {
	error("TODO: RLE");
}

void RLE_DrawDepth(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode) {
	error("TODO: RLE");
}

void RLE_DrawDepthRev(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode) {
	error("TODO: RLE");
}

void Raw_Draw(Buffer *src, Buffer *dest, int32 x, int32 y) {
	error("TODO: RLE");
}

void Raw_DrawRev(Buffer *src, Buffer *dest, int32 x, int32 y) {
	error("TODO: RLE");
}

void Raw_DrawDepth(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode) {
	error("TODO: RLE");
}

void Raw_DrawDepthRev(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode) {
	error("TODO: RLE");
}

void Raw_SDraw(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 *palette, uint8 *inverseColorTable) {
	error("TODO: RLE");
}

void Raw_SDrawRev(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 *palette, uint8 *inverseColorTable) {
	error("TODO: RLE");
}

void Raw_SDrawDepth(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, uint8 *palette, uint8 *inverseColorTable) {
	error("TODO: RLE");
}

void Raw_SDrawDepthRev(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, uint8 *palette, uint8 *inverseColorTable) {
	error("TODO: RLE");
}

void RLE_DrawOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset) {
	error("TODO: RLE");
}

void RLE_DrawDepthOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset) {
	error("TODO: RLE");
}

void RLE_DrawRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset) {
	error("TODO: RLE");
}

void RLE_DrawDepthRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset) {
	error("TODO: RLE");
}

void Raw_DrawOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset) {
	error("TODO: RLE");
}

void Raw_DrawRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset) {
	error("TODO: RLE");
}

void Raw_DrawDepthOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset) {
	error("TODO: RLE");
}

void Raw_DrawDepthRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset) {
	error("TODO: RLE");
}

void Raw_SDrawOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset, uint8 *palette, uint8 *inverseColorTable) {
	error("TODO: RLE");
}

void Raw_SDrawRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, int32 leftOffset, int32 rightOffset, uint8 *palette, uint8 *inverseColorTable) {
	error("TODO: RLE");
}

void Raw_SDrawDepthOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset, uint8 *palette, uint8 *inverseColorTable) {
	error("TODO: RLE");
}

void Raw_SDrawDepthRevOffs(Buffer *src, Buffer *dest, int32 x, int32 y, uint8 srcDepth, uint8 *depthCode, int32 leftOffset, int32 rightOffset, uint8 *palette, uint8 *inverseColorTable) {
	error("TODO: RLE");
}

} // namespace M4
