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

#ifndef IGOR_ROOM_TYPES_H
#define IGOR_ROOM_TYPES_H

#include "common/scummsys.h"

namespace Igor {

enum FacingPosition {
	kFacingPositionBack = 1,
	kFacingPositionRight = 2,
	kFacingPositionFront = 3,
	kFacingPositionLeft = 4
};

struct RoomWalkBounds {
	int x1, y1;
	int x2, y2;
	// Minimum y after clamping a click past the corresponding horizontal edge.
	// e.g. clamped to x1? then x1MinY is used to determine the minimum y.
	int x1MinY, x2MinY;
};

struct RoomObjectArea {
	uint8 area;
	uint8 object;
	uint8 y1Lum;
	uint8 y2Lum;
	uint8 deltaLum;
};

struct Action {
	uint8 verb;
	uint8 object1Num;
	uint8 object1Type;
	uint8 verbType; // 1:use,2:give
	uint8 object2Num;
	uint8 object2Type;
};

struct DialogueDataOffsets {
	int questionsOffset; // Base offset used to locate question text
	int questionsSize;
	int repliesOffset; // Base offset used to locate reply text
	int repliesSize;
	int matSize; // Stride used to index the conversation choices matrix
	int replyDataOffset;
	int questionSoundsOffset;
	int replySoundsOffset;
	int replySoundsSize;
};

struct RoomDataOffsets {
	struct {
		int box;
		int boxSize;
		int boxSrcSize;
		int boxDstSize;
	} area;
	struct {
		int walkPoints;
		int walkFacingPosition;
	} obj;
	struct {
		int defaultVerb;
		int useVerb;
		int giveVerb;
		int object2;
		int object1;
		int objectSize;
	} action;
	DialogueDataOffsets dlg;
};

// The click fix of a room that clamps the clicked position: y is limited, x is limited to [xMin, xMax]
// (-1: no limit), then the first walkable row below is looked for (and above if scanUp).
struct RoomWalkTargetRules {
	int yMax;
	int xMin;
	int xMax;
	bool scanUp;
	bool enabled;
};

struct DialogueText {
	int num;
	int count;
	int sound;
};

struct WalkData {
	int16 x, y;
	uint8 posNum;
	uint8 frameNum;
	uint8 clipSkipX;
	int16 clipWidth;
	int16 scaleWidth;
	uint8 xPosChanged;
	int16 dxPos;
	uint8 yPosChanged;
	int16 dyPos;
	uint8 scaleHeight;

	void setPos(int xPos, int yPos, uint8 facingPos, uint8 frame) {
		x = xPos;
		y = yPos;
		posNum = facingPos;
		frameNum = frame;
	}

	void setDefaultScale() {
		clipSkipX = 1;
		clipWidth = 30;
		scaleWidth = 50;
		xPosChanged = 1;
		dxPos = 0;
		yPosChanged = 1;
		dyPos = 0;
		scaleHeight = 50;
	}

	void setScale(int w, int h) {
		scaleWidth = w;
		scaleHeight = h;
	}

	static void setNextFrame(uint8 pos, uint8 &frame) {
		switch (pos) {
		case kFacingPositionBack:
		case kFacingPositionFront:
			if (frame == 6) {
				frame = 1;
			} else {
				++frame;
			}
			break;
		case kFacingPositionLeft:
		case kFacingPositionRight:
			if (frame == 8) {
				frame = 1;
			} else {
				++frame;
			}
			break;
		}
	}
};

} // End of namespace Igor

#endif // IGOR_ROOM_TYPES_H
