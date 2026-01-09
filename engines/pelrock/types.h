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

enum VerbIcon {
	PICKUP,
	TALK,
	WALK,
	LOOK,
	PUSH,
	PULL,
	OPEN,
	CLOSE,
	UNKNOWN,
	NO_ACTION
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

const int kAlfredFrameWidth = 51;
const int kAlfredFrameHeight = 102;

const int kChoiceHeight = 16; // Height of each choice line in pixels

const int kTalkAnimationSpeed = 2;   // Frames per update
const int kAlfredAnimationSpeed = 2; // Frames per update
const int kAlfredIdleAnimationFrameCount = 300;

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

#define MAX_CHARS_PER_LINE 0x2F // 47 characters
#define MAX_LINES 5             // Maximum number of lines per page (0-indexed check against 4)

#define ALFRED_COLOR 0x0D

#define OVERLAY_NONE 0
#define OVERLAY_CHOICES 1
#define OVERLAY_PICKUP_ICON 2
#define OVERLAY_ACTION 3

const byte kIconBlinkPeriod = 4;

enum AlfredAnimState {
	ALFRED_IDLE,
	ALFRED_WALKING,
	ALFRED_TALKING,
	ALFRED_INTERACTING,
	ALFRED_COMB
};

enum AlfredDirection {
	ALFRED_RIGHT = 0,
	ALFRED_LEFT = 1,
	ALFRED_DOWN = 2,
	ALFRED_UP = 3
};

struct ActionPopupState {
	bool isActive = false;
	int curFrame = 0;
	int x = 0;
	int y = 0;
	int displayTime = 0;
};

struct AlfredState {
	AlfredAnimState animState = ALFRED_IDLE;
	AlfredDirection direction = ALFRED_DOWN;
	int curFrame = 0;
	uint16 movementSpeed = 6; // pixels per frame
	uint16 x = 319;
	uint16 y = 302;
	uint16 scaledX = 0;
	uint16 scaledY = 0;
	int idleFrameCounter = 0;

	void setState(AlfredAnimState nextState) {
		animState = nextState;
		curFrame = 0;
	}
};

typedef struct {
	uint8_t flags;      /* Direction flags (see MOVE_* constants) */
	uint16_t distanceX; // Horizontal distance to move
	uint16_t distanceY; // Vertical distance to move
} MovementStep;

/**
 * Pathfinding context
 */
typedef struct {
	uint8_t *pathBuffer;          // Sequence of walkbox indices
	MovementStep *movementBuffer; // Array of movement steps
	uint8_t *compressed_path;     // Final compressed path
	uint16_t pathLength;
	uint16_t movementCount;
	uint16_t compressed_length;
} PathContext;

struct Anim {
	int16 x;
	int16 y;
	int w;
	int h;
	int nframes;
	int curFrame = 0;
	int curLoop = 0;
	byte **animData;
	byte loopCount;
	byte speed;
	byte elpapsedFrames = 0;
	uint16 movementFlags = 0;
};

struct Exit {
	byte index;
	int16 x;
	int16 y;
	byte w;
	byte h;
	uint16 targetRoom;
	int16 targetX;
	int16 targetY;
	uint16 targetDir;
	AlfredDirection dir;
	byte isEnabled;
};

struct Sprite {
	byte index; // number of the animation in the rooms
	byte type;
	int16 x;      // 0
	int16 y;      // 2
	int w;        // 4
	int h;        // 5
	byte extra;   // 6
	int numAnims; // 8
	int curAnimIndex = 0;
	int8 zOrder;
	byte spriteType;  // 33
	byte actionFlags; // 34
	bool isDisabled;  // 38
	bool isTalking = false;
	Anim *animData;
};

struct HotSpot {
	byte index;
	byte innerIndex;
	int id;
	int16 x;
	int16 y;
	int w;
	int h;
	byte actionFlags;
	int16 extra;
	bool isEnabled = true;
	bool isSprite = false;
	byte zOrder = 0;
};

struct TalkingAnims {
	uint32 spritePointer;

	byte unknown2[3];

	int8 offsetXAnimA;
	int8 offsetYAnimA;

	byte wAnimA;
	byte hAnimA;
	byte unknown3[2];
	byte numFramesAnimA;
	byte unknown4[5];

	byte offsetXAnimB;
	byte offsetYAnimB;
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

struct Description {
	byte itemId;
	byte index;
	bool isAction = false;
	uint16 actionTrigger;
	Common::String text;
};

struct WalkBox {
	byte index;
	int16 x;
	int16 y;
	int16 w;
	int16 h;
	byte flags;
};

struct QueuedAction {
	VerbIcon verb;
	int hotspotIndex;
	bool isQueued;
};

struct ScalingParams {
	int16 yThreshold;
	byte scaleDivisor;
	byte scaleMode;
};

struct ScaleCalculation {
	int scaledWidth;
	int scaledHeight;
	int scaleUp;
	int scaleDown;
};

enum GameState {
	GAME = 100,
	MENU = 101,
	CREDITS = 102,
	SAVELOAD = 103,
	SETTINGS = 104,
	EXTRA_SCREEN = 105,
	INTRO = 106,
};

struct HotSpotChange {
	byte roomNumber;
	byte hotspotIndex;
	HotSpot hotspot;
};

struct ExitChange {
	byte roomNumber;
	byte exitIndex;
	Exit exit;
};

struct WalkBoxChange {
	byte roomNumber;
	byte walkboxIndex;
	WalkBox walkbox;
};

struct InventoryObject {
	byte index;
	Common::String description;
	byte iconData[60 * 60];
};

struct PaletteAnimFade {
	byte startIndex;
	byte paletteMode;
	byte currentR;
	byte currentG;
	byte currentB;
	byte minR;
	byte minG;
	byte minB;
	byte maxR;
	byte maxG;
	byte maxB;
	byte speed;
	bool downDirection;
	byte curFrameCount = 0;
};

struct Sticker {
	int roomNumber;
	int stickerIndex;
	uint16 x;
	uint16 y;
	byte w;
	byte h;
	byte *stickerData;
};

struct PaletteAnimRotate {
	byte startIndex;
	byte paletteMode;
	byte unknown;
	byte delay;
	byte unknownBytes[7];
	byte flags;
	byte curFrameCount = 0;
};

struct PaletteAnim {
	byte startIndex;
	byte paletteMode;
	byte data[10]; // Based on mode its a rotate or fade
	byte curFrameCount = 0;
};

/**
 * Structure to hold a parsed choice option
 */
struct ChoiceOption {
	byte room;
	int choiceIndex;
	Common::String text;
	uint32 dataOffset;
	bool isDisabled;
	bool shouldDisableOnSelect = false;

	ChoiceOption() : choiceIndex(-1), isDisabled(false), dataOffset(0) {}
};

struct ResetEntry {
	uint16 room;
	uint16 offset;
	byte dataSize;
	byte *data = nullptr;
};


struct GameStateData {
	GameState stateGame = INTRO;

	Common::Array<byte> inventoryItems;
	int16 selectedInventoryItem = -1;
	Common::HashMap<byte, Common::Array<Sticker>> roomStickers;
	Common::HashMap<byte, Common::Array<ExitChange>> roomExitChanges;
	Common::HashMap<byte, Common::Array<WalkBoxChange>> roomWalkBoxChanges;
	Common::HashMap<byte, Common::Array<HotSpotChange>> roomHotSpotChanges;
	Common::HashMap<byte, Common::Array<ResetEntry>> disabledBranches;

	GameStateData() {
		memset(conversationRootsState, 0, 4 * 56);
	}

	~GameStateData() {
		delete[] conversationRootsState;
		conversationRootsState = nullptr;
	}

	void addDisabledBranch(ResetEntry entry) {
		disabledBranches[entry.room].push_back(entry);
	}

	byte *conversationRootsState = new byte[4 * 56];

	bool getRootDisabledState(byte room, byte root) const {
		return (conversationRootsState[room * 4 + root] != 0);
	}

	void setRootDisabledState(byte room, byte root, bool disabled) {
		conversationRootsState[room * 4 + root] = disabled ? 1 : 0;
	}
};

struct SaveGameData {
	byte currentRoom = 0;
	uint16 alfredX = 0;
	uint16 alfredY = 0;
	AlfredDirection alfredDir = ALFRED_DOWN;
	GameStateData *gameState = nullptr;
};

} // End of namespace Pelrock

#endif
