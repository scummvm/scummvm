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
#ifndef PELROCK_TYPES_H
#define PELROCK_TYPES_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/types.h"

namespace Pelrock {

enum Cursor {
	DEFAULT,
	HOTSPOT,
	EXIT,
	ALFRED,
	COMBINATION
};

enum VerbIcons {
	PICKUP,
	TALK,
	WALK,
	LOOK,
	PUSH,
	PULL,
	OPEN,
	CLOSE,
	UNKNOWN
};



static const uint32 kLongClickDuration = 500; // 500ms for long click
const int kCursorWidth = 16;
const int kCursorHeight = 18;
const int kCursorSize = 288; // 16 * 18
const int kRoomStructSize = 104;
const int kTalkingAnimHeaderSize = 55;
const int kNumRooms = 56;
const int kVerbIconWidth = 60;
const int kVerbIconHeight = 60;
const int kNumVerbIcons = 9;
const int kBalloonWidth = 247;
const int kBalloonHeight = 112;
const int kBalloonFrames = 4;
const int kTextCharDisplayTime = 100; // 10ms per character
const int kVerbIconPadding = 20;

// Direction flags (bit-packed)
#define MOVE_RIGHT 0x01 // Move right (positive X)
#define MOVE_LEFT 0x02  // Move left (negative X)
#define MOVE_HORIZ 0x03 // Horizontal movement mask
#define MOVE_DOWN 0x04  // Move down (positive Y)
#define MOVE_UP 0x08    // Move up (negative Y)
#define MOVE_VERT 0x0C  // Vertical movement mask
#define MAX_PATH_LENGTH 100
#define MAX_MOVEMENT_STEPS 100 // 500 bytes / 5 bytes per step
#define PATH_END 0xFF          // End of path marker

#define MAX_CHARS_PER_LINE  0x2F  // 47 characters
#define MAX_LINES 5  // Maximum number of lines per page (0-indexed check against 4)

// Control character codes (negative values in signed char)
#define CHAR_SPACE 0x20           /* ' ' */
#define CHAR_END_MARKER_1 0xFD    /* -3 (end of text marker) */
#define CHAR_END_MARKER_2 0xF4    /* -0xC (alternate end marker) */
#define CHAR_END_MARKER_3 0xF8    /* -8 (another end marker) */
#define CHAR_END_MARKER_4 0xF0    /* -0x10 (another end marker) */
#define CHAR_NEWLINE 0xF6         /* -10 (newline marker) */
#define CHAR_PAGE_BREAK 0xF9      /* marker inserted when switching pages */

#define ALFRED_COLOR 0x0D


typedef struct {
	uint8_t flags;       /* Direction flags (see MOVE_* constants) */
	uint16_t distance_x; // Horizontal distance to move
	uint16_t distance_y; // Vertical distance to move
} MovementStep;

/**
 * Pathfinding context
 */
typedef struct {
	uint8_t *path_buffer;          // Sequence of walkbox indices
	MovementStep *movement_buffer; // Array of movement steps
	uint8_t *compressed_path;      // Final compressed path
	uint16_t path_length;
	uint16_t movement_count;
	uint16_t compressed_length;
} PathContext;

struct Anim {
	int x;
	int y;
	int w;
	int h;
	int nframes;
	int curFrame = 0;
	int curLoop = 0;
	byte *animData;
	byte loopCount;
	byte speed;
	byte elpapsedFrames = 0;
};

struct Exit {
	uint16 x;
	uint16 y;
	byte w;
	byte h;
	uint16 targetRoom;
	uint16 targetX;
	uint16 targetY;
	uint16 targetDir;
	byte dir;
	byte flags;
};

struct AnimSet {
	byte type;
	int x;        // 0
	int y;        // 2
	int w;        // 4
	int h;        // 5
	byte extra;   // 6
	int numAnims; // 8
	int curAnimIndex = 0;
	byte spriteType;  // 33
	byte actionFlags; // 34
	bool isDisabled;  // 38
	Anim *animData;
};

struct HotSpot {
	int index;
	int id;
	int x;
	int y;
	int w;
	int h;
	byte type;
	int extra;
	bool isEnabled = true;
};

struct TalkinAnimHeader {
	uint16 spritePointer;
	byte unknown1;
	byte unknown2[4];

	int8 offsetX;
	int8 offsetY;

	byte wAnimA;
	byte hAnimA;
	byte unknown3[2];
	byte numFramesAnimA;
	byte unknown4[7];

	byte currentFrameAnimA;

	byte wAnimB;
	byte hAnimB;
	byte unknown5;
	byte numFramesAnimB;
	byte unknown6[29];
	byte currentFrameAnimB;

	byte **animA = nullptr;
	byte **animB = nullptr;
};

struct ConversationElement {
	enum Type {
		DIALOGUE,
		CHOICE_MARKER,
		END_CONV,
		END_BRANCH
	} type;

	Common::String speaker;
	byte speakerId;
	Common::String text;
	int choiceIndex;
	bool isRealChoice;

	ConversationElement() : type(DIALOGUE), choiceIndex(-1), isRealChoice(false) {}
};

struct ConversationNode {
	enum NodeType {
		ROOT,
		CHOICE,
		RESPONSE
	} type;

	Common::String text;
	Common::String speaker;
	byte speakerId;
	int choiceIndex;
	bool terminated;

	Common::Array<ConversationNode> choices;
	Common::Array<ConversationNode> responses;
	Common::Array<ConversationNode> subchoices;

	ConversationNode() : type(ROOT), choiceIndex(-1), terminated(false) {}
};

struct StackEntry {
	ConversationNode *node;
	int index;
};

struct Description {
	byte itemId;
	byte index;
	bool isAction = false;
	uint16 actionTrigger;
	Common::String text;
};

struct WalkBox {
	int16 x;
	int16 y;
	int16 w;
	int16 h;
	byte flags;
};

enum GameState {
	GAME = 100,
	MENU = 101,
	CREDITS = 102,
	SAVELOAD = 103,
	SETTINGS = 104,
	EXTRA_SCREEN = 105,
	INTRO = 106,
	PROMOTE = 107,
};

} // End of namespace Pelrock

#endif
