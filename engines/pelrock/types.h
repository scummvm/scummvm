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

#define ACTION_MASK_NONE 0
#define ACTION_MASK_OPEN 1
#define ACTION_MASK_CLOSE 2
#define ACTION_MASK_UNKNOWN 4
#define ACTION_MASK_PICKUP 8
#define ACTION_MASK_TALK 16
#define ACTION_MASK_PUSH 32
#define ACTION_MASK_PULL 128

enum VerbIcon {
	PICKUP,
	TALK,
	WALK,
	LOOK,
	PUSH,
	PULL,
	OPEN,
	CLOSE,
	ITEM,
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

const int kTalkAnimationSpeed = 2; // Frames per update
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
	ALFRED_COMB,
	ALFRED_SPECIAL_ANIM
};

enum AlfredDirection {
	ALFRED_RIGHT = 0,
	ALFRED_LEFT = 1,
	ALFRED_DOWN = 2,
	ALFRED_UP = 3
};

struct AlfredSpecialAnim {
	byte *animData = nullptr;
	int w = 0;
	int h = 0;
	int numFrames = 0;
	int loopCount = 0;
	uint32 stride = 0;
	int curFrame = 0;
	int curLoop = 0;
	uint32 size = 0;
	AlfredSpecialAnim(int nF, int width, int height, int nBudas, uint32 off, int loopCount, uint32 sz)
		: numFrames(nF), w(width), h(height), loopCount(loopCount), size(sz) {
		stride = w * h;
	}
	~AlfredSpecialAnim() {
		if (animData) {
			delete[] animData;
			animData = nullptr;
		}
	}
};

struct ActionPopupState {
	bool isActive = false;
	int curFrame = 0;
	int x = 0;
	int y = 0;
	int displayTime = 0;
	bool isAlfredUnder = false;
};

struct AlfredState {
	AlfredAnimState animState = ALFRED_IDLE;
	AlfredDirection direction = ALFRED_DOWN;
	int curFrame = 0;
	uint16 movementSpeedX = 6; // pixels per frame
	uint16 movementSpeedY = 5; // pixels per frame
	uint16 x = 319;
	uint16 y = 302;
	byte w = kAlfredFrameWidth;
	byte h = kAlfredFrameHeight;
	int idleFrameCounter = 0;
	bool isWalkingCancelable = true;

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
	int16 x;       // 0
	int16 y;       // 2
	int w;         // 4
	int h;         // 5
	uint16 stride; // 6-7
	int numAnims;  // 8
	int curAnimIndex = 0;
	int8 zOrder; //32-33

	byte actionFlags;       // 34
	bool isHotspotDisabled; // 38
	bool isTalking = false;
	Anim *animData;
	int16 extra;
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
	int scaleX; // Amount to subtract from width (was scaleUp)
	int scaleY; // Amount to subtract from height (was scaleDown)
};

enum GameState {
	GAME = 100,
	CREDITS = 101,
	SETTINGS = 102,
	INTRO = 103,
	COMPUTER = 104
};

struct SpriteChange
{
	byte roomNumber;
	byte spriteIndex;
	byte zIndex;

};


struct HotSpotChange {
	byte roomNumber;
	byte hotspotIndex;
	HotSpot hotspot;
};

struct ExitChange {
	byte roomNumber;
	byte exitIndex;
	bool enabled;
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
	byte curFrame = 0;
	byte tickCount = 0;
};

#define PASSERBY_RIGHT 0
#define PASSERBY_LEFT 1
#define PASSERBY_DOWN 2
struct PasserByAnim
{
	uint32 frameTrigger = 0x3FF;
	int16 startX;
	int16 startY;
	int16 resetCoord;
	byte dir;
	byte spriteIndex;
	byte targetZIndex;
};

struct RoomPasserBys {
	byte roomNumber;
	PasserByAnim passerByAnims[2];
	byte currentAnimIndex = 0;
	byte numAnims = 0;
	bool latch = false;
	RoomPasserBys(byte roomNum, byte numAnims) : roomNumber(roomNum), numAnims(numAnims) {}
};


/**
 * Structure to hold a parsed choice option
 */
struct ChoiceOption {
	byte room;
	int choiceIndex;
	Common::String text;
	uint32 dataOffset;
	bool isDisabled = false;
	bool shouldDisableOnSelect = false;
	bool hasConversationEndMarker = false;
	bool isTerminator = false;

	ChoiceOption() : choiceIndex(-1), dataOffset(0) {}
};

struct ResetEntry {
	uint16 room;
	uint16 offset;
	byte dataSize;
	byte *data = nullptr;
};

#define FLAG_JEFE_INGRESA_PASTA 0
#define FLAG_JEFE_ENCARCELADO 1
#define FLAG_PUESTA_SALSA_PICANTE 2
#define FLAG_CRISTAL_ROTO 3
#define FLAG_ENTRA_EN_TIENDA_PRIMERA_VEZ 4
#define FLAG_ELECTROCUTACION 5
#define FLAG_CABLES_PUESTOS 6
#define FLAG_SOBORNO_PORTERO 7
#define FLAG_MEMORIZA_LIBRO 8
#define FLAG_ALFRED_INTELIGENTE 9
#define FLAG_ALFRED_SABE_EGIPCIO 10
#define FLAG_VENDEDOR_DEJA_DE_JODER 11

#define FLAG_VIAJE_A_EGIPTO 12
#define FLAG_PARADOJA_RESUELTA 13
#define FLAG_CROCODILLO_ENCENDIDO 14
#define FLAG_MIRA_SIMBOLO_FUERA_MUSEO 15
#define FLAG_PUERTA_SECRETA_ABIERTA 16
#define FLAG_ROBA_PELO_PRINCESA 17
#define FLAG_A_LA_CARCEL 18
#define FLAG_CLAVE_CAJA_FUERTE 19
#define FLAG_SE_HA_PUESTO_EL_MUNECO 20
#define FLAG_VIGILANTE_BEBE_AGUA 21
#define FLAG_VIGILANTE_MEANDO 22
#define FLAG_PIRAMIDE_JODIDA 23
#define FLAG_PIRAMIDE_JODIDA2 24
#define FLAG_VIGILANTE_PAJEANDOSE 25
#define FLAG_FORMULA_MAGICA 26
#define FLAG_VIAJA_AL_PASADO 27
#define FLAG_APARECE_EUNUCO 28
#define FLAG_AL_FARAON 29
#define FLAG_A_CURRAR 30
#define FLAG_DA_PIEDRA 31
#define FLAG_PIEDRAS_COGIDAS 32
#define FLAG_GUARDIAS_BORRACHOS 33
#define FLAG_PIEDRA_FAKE_MOJADA 34
#define FLAG_PUERTA_BUENA 35
#define FLAG_TRAMPILLA_ABIERTA 36
#define FLAG_HABITACION_PRINCESA 37
#define FLAG_A_POR_LA_PRINCESA 38
#define FLAG_VUELTA_A_EMPEZAR 39
#define FLAG_A_LOS_PASILLOS 40
#define FLAG_COMO_ESTAN_LOS_DIOSES 41
#define FLAG_END_OF_GAME 42
#define FLAG_FROM_INTRO 43
#define FLAG_HE_TIRADO_PIEDRA 44
#define FLAG_HA_USADO_AGUA 45
#define FLAG_TIENDA_ABIERTA 46
#define FLAG_NUMERO_DE_COPAS 47
#define FLAG_INGREDIENTES_CONSEGUIDOS 48

#define FLAG_GUARDIA_PIDECOSAS 49
#define FLAG_GUARDIA_DNI_ENTREGADO 50
#define FLAG_AGENCIA_ABIERTA 51
#define FLAG_CONSIGNAS_VENDEDOR 52
#define FLAG_PUTA_250_VECES 53
#define FLAG_RESPUESTAS_ACERTADAS 54
#define FLAG_CHEAT_CODE_ENABLED 55    // 0x495F3 - enables HIJODELAGRANPUTA cheat code input
#define FLAG_RIDDLE_SOLVED 56         // 0x495D0 - set when Egyptian riddle answered correctly

const int kNumGameFlags = 57;

struct GameStateData {
	byte flags[kNumGameFlags];

	GameState stateGame = INTRO;

	Common::Array<byte> inventoryItems;
	int16 selectedInventoryItem = -1;

	int libraryShelf = -1;
	int selectedBookIndex = -1;
	unsigned char bookLetter = '\0';
	Common::HashMap<byte, Common::Array<Sticker>> stickersPerRoom;
	Common::HashMap<byte, Common::Array<ExitChange>> roomExitChanges;
	Common::HashMap<byte, Common::Array<WalkBoxChange>> roomWalkBoxChanges;
	Common::HashMap<byte, Common::Array<HotSpotChange>> roomHotSpotChanges;
	Common::HashMap<byte, Common::Array<ResetEntry>> disabledBranches;
	Common::HashMap<byte, Common::Array<SpriteChange>> spriteChanges;

	GameStateData() {
		memset(conversationCurrentRoot, 0xFF, 56); // 0xFF = not set
		for (int i = 0; i < kNumGameFlags; i++)
			flags[i] = 0;
		flags[FLAG_ENTRA_EN_TIENDA_PRIMERA_VEZ] = true;
	}

	~GameStateData() {
		delete[] conversationCurrentRoot;
		conversationCurrentRoot = nullptr;
	}

	void addDisabledBranch(ResetEntry entry) {
		disabledBranches[entry.room].push_back(entry);
	}

	byte getFlag(int flagIndex) const {
		if (flagIndex < 0 || flagIndex >= kNumGameFlags)
			return false;
		return flags[flagIndex];
	}

	void setFlag(int flagIndex, byte value) {
		if (flagIndex < 0 || flagIndex >= kNumGameFlags)
			return;
		flags[flagIndex] = value;
	}

	void addInventoryItem(int id) {
		inventoryItems.push_back(id);
	}

	void removeInventoryItem(int id) {
		for (uint i = 0; i < inventoryItems.size(); i++) {
			if (inventoryItems[i] == id) {
				inventoryItems.remove_at(i);
				return;
			}
		}
	}

	bool hasInventoryItem(int id) const {
		for (uint i = 0; i < inventoryItems.size(); i++) {
			if (inventoryItems[i] == id) {
				return true;
			}
		}
		return false;
	}

	// Store current root index for each room (0xFF = not set, use findRoot logic)
	byte *conversationCurrentRoot = new byte[56];

	int getCurrentRoot(byte room) const {
		if (room >= 56)
			return -1;
		return (conversationCurrentRoot[room] == 0xFF) ? -1 : conversationCurrentRoot[room];
	}

	void setCurrentRoot(byte room, int root) {
		if (room >= 56)
			return;
		if (root < 0 || root > 254) {
			conversationCurrentRoot[room] = 0xFF; // Reset to auto-select
		} else {
			conversationCurrentRoot[room] = (byte)root;
		}
	}

	int findFirstBookIndex() {
		for (uint i = 0; i < inventoryItems.size(); i++) {
			int x = inventoryItems[i];
			if ((x >= 11) && (x <= 58))
				return x;
		}
		return -1;
	}


	int booksInInventory() {
		int l = inventoryItems.size();
		int count = 0;
		for (int i = 0; i < l; i++) {
			int x = inventoryItems[i];
			if ((x >= 11) && (x <= 58))
				count++;
		}
		return count;
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
