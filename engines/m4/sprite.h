/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef M4_SPRITE_H
#define M4_SPRITE_H

#include "common/util.h"
#include "common/endian.h"
#include "common/stream.h"
#include "m4/graphics.h"

/*
	TODO:
		- change DrawRequestX and RendCell
*/

namespace M4 {

typedef struct
{
	int32	 x;		// x position relative	to GrBuff(0, 0)
	int32	 y;		// y position relative	to GrBuff(0, 0)
	int32	 scale_x;		// x scale factor (can be negative for reverse draw)
	int32	 scale_y;		// y scale factor (can't be negative)
	uint8*	depth_map;	// depth code array for destination (doesn't care if srcDepth is 0)
	BGR8	  *Pal;		// palette for shadow draw (doesn't care if SHADOW bit is not set in Src.encoding)
	uint8*	ICT;		// Inverse Color Table (doesn't care if SHADOW bit is not set in Src.encoding)
	uint8	 depth;		// depth code for source (0 if no depth processing)
} DrawRequestX;

typedef struct
{
	uint32 Pack;
	uint32 Stream;
	long   hot_x;
	long   hot_y;
	uint32 Width;
	uint32 Height;
	uint32 Comp;
	uint32 Reserved[8];
	uint8* data;
} RendCell;

#define SS_HEADER_NUM_FIELDS 14
struct SpriteSeriesHeader {
	uint32 header;
	uint32 size;
	uint32 packing;
	uint32 frameRate;
	uint32 pixSpeed;
	uint32 maxWidth;
	uint32 maxHeight;
	uint32 reserved3;
	uint32 reserved4;
	uint32 reserved5;
	uint32 reserved6;
	uint32 reserved7;
	uint32 reserved8;
	uint32 count;
};

#define SF_HEADER_NUM_FIELDS 15
struct SpriteFrameHeader {
	uint32 pack;
	uint32 stream;
	uint32 x;
	uint32 y;
	uint32 width;
	uint32 height;
	uint32 comp;
	uint32 reserved1;
	uint32 reserved2;
	uint32 reserved3;
	uint32 reserved4;
	uint32 reserved5;
	uint32 reserved6;
	uint32 reserved7;
	uint32 reserved8;
};

class M4Sprite: public M4Surface {
public:
	int x, y;
	int xOffset, yOffset;
	uint8 encoding;

	M4Sprite(M4Engine *vm): M4Surface() {}
	M4Sprite(M4Engine *vm, int widthVal, int heightVal): M4Surface(widthVal, heightVal), xOffset(0), yOffset(0) {}
	// Loads a sprite from the given stream, and optionally decompresses the RLE-encoded data
	M4Sprite(Common::SeekableReadStream* source, int xOfs, int yOfs, int widthVal, int heightVal, bool decodeRle = true, uint8 encodingVal = 0);
	// Loads an RLE compressed sprite; the surface must have been created before
	void loadRle(Common::SeekableReadStream* rleData);
	void loadDeltaRle(Common::SeekableReadStream* rleData, int destX, int destY);
	void loadMadsSprite(Common::SeekableReadStream* source);

	byte getTransparentColor() const;
protected:
};

} // End of namespace M4

#endif
