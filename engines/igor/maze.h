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

#ifndef IGOR_MAZE_H
#define IGOR_MAZE_H

#include "common/scummsys.h"
#include "igor/room_types.h"

namespace Igor {

struct MazeNode {
	uint8 neighbors[4];
	uint8 shape;
};

enum MazeDirection {
	kMazeNorth,
	kMazeEast,
	kMazeSouth,
	kMazeWest
};

enum MazeEntryKind {
	kMazeEntryWalk,     // Igor appears at (x, y) and walks to (destX, destY)
	kMazeEntryStairs    // Igor walks into the picture at x, scaling down
};

struct MazeEntry {
	uint16 state;
	uint8 kind;
	int16 x, y;
	uint8 facing;
	int16 destX, destY;
};

enum MazeActionKind {
	kMazeActionExit,        // walk (x1, y1) -> (x2, y2), then to the neighbour location
	kMazeActionStairsExit,  // walk out of the picture at x, scaling up, then to the neighbour location
	kMazeActionExitFixed,   // walk, then to a fixed location and state
	kMazeActionDialogue
};

struct MazeDialogueLine {
	uint16 text;
	uint8 count;
	uint16 sound;
};

struct MazeAction {
	uint8 code;
	uint8 kind;
	int16 x1, y1, x2, y2;
	uint8 dir;
	int16 stairsX;
	int16 location;     // -1: unchanged
	int16 state;
	int8 objectState;   // -1: none
	int8 objectStateValue;
	const MazeDialogueLine *lines;
	uint8 numLines;
	uint8 dialogueStart;
	uint8 dialogueCount;
};

enum MazeFlameMode {
	kMazeFlameNone,
	kMazeFlameFixed,        // one flame at a fixed position, color 1 shows the first frame
	kMazeFlameFirstFrame,   // color 1 shows the first frame
	kMazeFlameLayer1        // color 1 shows the next pixel of the room
};

struct MazeFlame {
	int16 x, y;
};

struct MazeRoom {
	uint8 part;
	int dat, txt, img, pal, msk, box;
	int8 music;
	uint8 darkness;
	uint8 flameMode;
	uint8 numFlames;
	MazeFlame flames[2];
	uint8 flameFrames[2];
	uint8 secondFlameFrame;
	bool saveLocation;      // the room ends when the location changes
	const RoomDataOffsets *offsets;
	int giveObjectSize;
	RoomWalkTargetRules clickFix;
	const MazeEntry *entries;
	uint8 numEntries;
	const MazeAction *actions;
	uint8 numActions;
	const char *objectName3;
};

} // End of namespace Igor

#endif // IGOR_MAZE_H
