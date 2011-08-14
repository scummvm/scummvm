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

#include "common/endian.h"

struct Sprite {
	uint16 _updateCallback;
	uint16 updateCallback() const { return READ_LE_UINT16(&_updateCallback); }
	void setUpdateCallback(uint16 v) { WRITE_LE_UINT16(&_updateCallback, v); }
	uint16 w2;
	uint16 w4;
	uint16 _frameData;
	uint16 frameData() const { return READ_LE_UINT16(&_frameData); }
	void setFrameData(uint16 v) { WRITE_LE_UINT16(&_frameData, v); }
	uint16 w8;
	uint8  x;
	uint8  y;
	uint16 w12;
	uint8  b14;
	uint8  b15;
	uint8  b16;
	uint8  b17;
	uint8  delay;
	uint8  frame;
	uint16 _objData;
	uint16 objData() const { return READ_LE_UINT16(&_objData); }
	void setObjData(uint16 v) { WRITE_LE_UINT16(&_objData, v); }
	uint8  b22;
	uint8  priority;
	uint16 w24;
	uint16 w26;
	uint8  b28;
	uint8  b29;
	uint8  type;
	uint8  hidden;
};

struct ObjData {
	uint8 b0;
	uint8 b1;
	uint8 b2;
	uint8 b3;
	uint8 b4;
	uint8 b5;
	uint8 b6;
	uint8 delay;
	uint8 type;
	uint8 b9;
	uint8 b10;
	uint8 b11;
	uint8 b12;
	uint8 b13;
	uint8 b14;
	uint8 b15;
	uint8 b16;
	uint8 b17;
	uint8 b18[256]; // NB: Don't know the size yet
};

struct Frame {
	uint8  width;
	uint8  height;
	uint16 _ptr;
	uint16 ptr() const { return READ_LE_UINT16(&_ptr); }
	void setPtr(uint16 v) { WRITE_LE_UINT16(&_ptr, v); }
	uint8  x;
	uint8  y;
};

struct Reel {
	uint8 frame_lo;
	uint8 frame_hi;
	uint16 frame() const { return READ_LE_UINT16(&frame_lo); }
	void setFrame(uint16 v) { WRITE_LE_UINT16(&frame_lo, v); }
	uint8 x;
	uint8 y;
	uint8 b4;
};

struct Room {
	uint8 name[10];
	uint8 b10;
	uint8 b11;
	uint8 b12;
	uint8 roomsSample;
	uint8 b14;
	uint8 mapX;
	uint8 mapY;
	uint8 b17;
	uint8 b18;
	uint8 b19;
	uint8 liftFlag;
	uint8 b21;
	uint8 b22;
	uint8 countToOpen;
	uint8 liftPath;
	uint8 doorPath;
	uint8 b26;
	uint8 b27;
	uint8 b28;
	uint8 b29;
	uint8 b30;
	uint8 b31;
};

struct Rain {
	uint8 x;
	uint8 y;
	uint8 size;
	uint8 w3_lo;
	uint8 w3_hi;
	uint16 w3() const { return READ_LE_UINT16(&w3_lo); }
	void setW3(uint16 v) { WRITE_LE_UINT16(&w3_lo, v); }
	uint8 b5;
};

