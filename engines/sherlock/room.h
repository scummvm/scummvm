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
 */

#ifndef SHERLOCK_ROOM_H
#define SHERLOCK_ROOM_H

#include "common/scummsys.h"
#include "sherlock/sprite.h"

namespace Sherlock {

#define ROOMS_COUNT 63

/*
struct RoomBounds {
	int x, y, width, height;
};

struct BgshapeSub {
	uint16 value;
};

struct Bgshape {
	char name[12];
	char description[41];
	char *textBufferPtr;
	byte *seqBufPtr;
	Sprite *sprite;
	SpriteFrame *spriteFrame;
	byte byte05;
	byte seqBigCountFlag;
	int16 seqIndex;
	int16 canimIndex;
	int16 x, y;
	int16 xIncr, yIncr,
	uint16 status;
	int16 x2, y2;
	int16 width2, height2;
	uint16 word02;
	uint16 word03;
	byte flag;
	byte itemValue;
	uint16 word01;
	uint16 word05;
	uint16 stringIndex;
	int16 width, height;
	uint16 word04;
	byte flagsAndIndex;
	uint16 frameCount;
	byte spriteFlags;
	char string1[50];
	byte byte07;
	byte byte01;
	byte byte02;
	int16 boundsX, boundsY;
	byte direction;
	byte animIndex;
	char string2[50];
	byte byte06;
	byte seqByte;
	uint16 textBufferOfs;
	byte byte03;
	uint16 framesCopyCount;
	byte byte08;
	char string3[51];
	uint16 word06;
	uint16 word07;
	uint16 word08;
	uint16 word09;
	BgshapeSub subItems[4];
};
*/
class Rooms {
public:
	bool _stats[ROOMS_COUNT][9];
	bool _savedStats[ROOMS_COUNT][9];
public:
	Rooms();
};

} // End of namespace Sherlock

#endif
