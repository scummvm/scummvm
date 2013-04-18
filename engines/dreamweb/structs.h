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

struct RectWithCallback {
	uint16 _xMin, _xMax;
	uint16 _yMin, _yMax;
	uint16 _callback;

	uint16 xMin() const { return READ_LE_UINT16(&_xMin); }
	uint16 xMax() const { return READ_LE_UINT16(&_xMax); }
	uint16 yMin() const { return READ_LE_UINT16(&_yMin); }
	uint16 yMax() const { return READ_LE_UINT16(&_yMax); }
	uint16 callback() const { return READ_LE_UINT16(&_callback); }

	bool contains(uint16 x, uint16 y) const {
		return (x >= xMin()) && (x < xMax()) && (y >= yMin()) && (y < yMax());
	}
};

struct SetObject {
	uint8 b0;
	uint8 b1;
	uint8 b2;
	uint8 b3;
	uint8 b4;
	uint8 priority;
	uint8 b6;
	uint8 delay;
	uint8 type;
	uint8 b9;
	uint8 b10;
	uint8 b11;
	uint8 name[4];
	uint8 b16;
	uint8 index;
	uint8 b18[13]; // NB: Don't know the size yet
	uint8 b31;
	uint8 b32;
	uint8 b33;
	uint8 b34;
	uint8 b35;
	uint8 b36;
	uint8 b37;
	uint8 b38;
	uint8 b39;
	uint8 b40;
	uint8 b41;
	uint8 b42;
	uint8 b43;
	uint8 b44;
	uint8 b45;
	uint8 b46;
	uint8 b47;
	uint8 b48;
	uint8 b49;
	uint8 b50;
	uint8 b51;
	uint8 b52;
	uint8 b53;
	uint8 b54;
	uint8 b55;
	uint8 b56;
	uint8 b57;
	uint8 mapad[5];
	uint8 b63;
};

struct DynObject {
	uint8 currentLocation;
	uint8 index;
	uint8 mapad[5];
	uint8 b7;
	uint8 b8;
	uint8 b9;
	uint8 b10;
	uint8 initialLocation;
	uint8 id[4];
};

struct ObjPos {
	uint8 xMin;
	uint8 yMin;
	uint8 xMax;
	uint8 yMax;
	uint8 index;
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

struct ReelRoutine {
	uint8 reallocation;
	uint8 mapX;
	uint8 mapY;
	uint8 b3;
	uint8 b4;
	uint16 reelPointer() const { return READ_LE_UINT16(&b3); }
	void setReelPointer(uint16 v) { WRITE_LE_UINT16(&b3, v); }
	uint8 b5;
	uint8 b6;
	uint8 b7;
};

struct People {
	uint8 b0;
	uint8 b1;
	uint16 reelPointer() const { return READ_LE_UINT16(&b0); }
	void setReelPointer(uint16 v) { WRITE_LE_UINT16(&b0, v); }
	uint8 b2;
	uint8 b3;
	uint16 routinePointer() const { return READ_LE_UINT16(&b2); }
	void setRoutinePointer(uint16 v) { WRITE_LE_UINT16(&b2, v); }
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

struct Change {
	uint8 index;
	uint8 location;
	uint8 value;
	uint8 type;
};

struct PathNode {
	uint8 x;
	uint8 y;
	uint8 b2;
	uint8 b3;
	uint8 b4;
	uint8 b5;
	uint8 on;
	uint8 dir;
};

struct PathSegment {
	uint8 b0;
	uint8 b1;
};

struct RoomPaths {
	PathNode    nodes[12];
	PathSegment segments[24];
};

