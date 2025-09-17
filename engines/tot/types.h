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
#ifndef TOT_TYPES_H
#define TOT_TYPES_H

#include "common/scummsys.h"

namespace Tot {

const int kVerbRegSize = 263;
const int kRoomRegSize = 10856;
const int kItemRegSize = 279;

const int kList1Index = 19;
const int kList2Index = 20;
const int kList1code = 149;
const int kList2code = 150;
/**
 * Num of icons in the inventory
 */
const int kInventoryIconCount = 34;
const int kNumScreenOverlays = 15;
/**
 * Num of depth levels
 */
const int kDepthLevelCount = 15;
/**
 * Num of frames of the main character in a single direction
 */
const int kWalkFrameCount = 16;
/**
 * Num of frames in the secondary animation
 */
const int kSecAnimationFrameCount = 50;
/**
 * X factor of the screen grid
 * This results in 320/8 = 40 quadrants.
 */
const int kXGridCount = 8;
/**
 * Y factor of the screen grid
 * This results in 140/5 = 28 quadrants.
 */
const int kYGridCount = 5;
/**
 * Y offset of feet within character sprite (for adjustment of path finding)
 */
const int kCharacerCorrectionY = 49;
/**
 * X offset of feet within character sprite (for adjustment of path finding)
 */
const int kCharacterCorrectionX = 14;
const int kObjectNameLength = 20;
const int kCharacterCount = 9;
/**
 * Size of an inventory icon (39x26)
 */
const int kInventoryIconSize = 1018;
/**
 * Number of points in a
 */
const int kRoutePointCount = 7;

/**
 * Trajectory changes
 */
typedef Common::Point Route[kRoutePointCount];


enum TRAJECTORIES_OP {
	// Subtracts the animation dimensions to the trajectory coordinates to adjust movement
	SET_WITH_ANIM = false,
	// Adds the animation dimensions to the trajectory coordinates to restore
	RESTORE = true
};

struct ObjectInfo {
	uint16 code, posx, posy, posx2, posy2;
};

struct CharacterAnim {
	uint16 depth;
	byte *bitmap[4][kWalkFrameCount + 30]; // 30 = 3 actions * 10 frames each
};

struct SecondaryAnim {
	uint16 depth, dir, posx, posy;
	byte *bitmap[4][kSecAnimationFrameCount];
};

struct InventoryEntry {
	uint16 bitmapIndex;
	uint16 code;
	Common::String objectName;
};

/**
 * Hypertext struct
 */
struct TextEntry {
	Common::String  text; // string
	bool 			continued = false;  // true if the next entry is a continuation of this one
	uint16 			response = 0;   // entry number of reply
	int32 			pointer = 0;
};

struct ScreenObject {
	/**
	 * registry number
	 */
	uint16 code;
	byte height;             /* 0 top 1 middle 2 bottom, determines character anim on use/pick*/
	Common::String name;     /* name for mouseover*/
	uint16 lookAtTextRef;    /* Text reference when looking at object */
	uint16 beforeUseTextRef; /* Text reference before using object */
	uint16 afterUseTextRef;  /* Text reference after using object */
	uint16 pickTextRef;      /* Text reference when picking up object */
	uint16 useTextRef;       /* Text reference when using object */
	byte speaking;              /* whether the object talks or not */
	bool openable;           /* true if it can be opened */
	bool closeable;          /* true if it can be closed*/
	byte used[8];			 /* flags (one per original savegame) on whether the object has been used */
	bool pickupable;
	uint16 useWith,
		replaceWith; /* Code of the object it should be replaced with in case a replacement is due.*/
	byte depth;
	uint32 bitmapPointer; /* reference to the objects bitmap in the bitmap resource file */
	uint16 bitmapSize;
	uint32 rotatingObjectAnimation; /* Pointer to the FLC animation of the rotatin object */
	uint16 rotatingObjectPalette; /* Pointer to the palette of the above FLC animation*/
	uint16 dropOverlayX, dropOverlayY; /* coords when the object requires placing an overlay on the screen */
	uint32 dropOverlay; /* pointer to such overlay */
	uint16 dropOverlaySize;
	uint16 objectIconBitmap; /* Icon on the inventory */

	byte xgrid1, ygrid1, xgrid2, ygrid2; /* position of patches below*/

	byte walkAreasPatch[10][10]; /* patch on the scene's walking area (e.g. object prevents character from walking */
	byte mouseGridPatch[10][10]; /* patch on the mouse grid area (i.e. selectable area of the object */
};

struct DoorRegistry {
	uint16 	nextScene,
			exitPosX,
			exitPosY;
	byte	openclosed,
			doorcode;
};

struct RoomObjectListEntry {
	uint16 fileIndex;
	Common::String objectName;
};

struct RoomBitmapRegister {
	uint32 bitmapPointer;
	uint16 bitmapSize;
	uint16 coordx, coordy, depth;
};

struct RoomFileRegister {
	uint16 code;
	uint32 roomImagePointer;
	uint16 roomImageSize;
	byte walkAreasGrid[40][28]; /* movement grid */
	byte mouseGrid[40][28];    /* mousegrid with index to indexadoObjetos */
	/**
	 * This is a preset matrix of trajectories from different areas of the game.action
	 * Each room is divided into a number of areas according to the screen grid [rejapantalla].action
	 *
	 * A given coordinate in the game (x,y) is mapped into a grid position by dividing the x with factorx (8)
	 * and the y coordinate by factory (5). With each room being 320x140 this results in
	 * 40 horizontal divisions and 28 vertical divisions which matches the [rejapantalla] matrix.action
	 *
	 * When obtaining the grid position a given (x,y) coordinate matches to, [rejapantalla] returns an
	 * area number.action
	 *
	 * trayectorias then has a precalculated route from each possible combination of two areas in the game.
	 * pixel by pixel translation within the area is done by bresenham algorithm in the trajectory function.
	 */
	Common::Point trajectories[9][30][5];
	DoorRegistry doors[5]; /* doors in the room */
	RoomBitmapRegister screenLayers[15];
	RoomObjectListEntry *screenObjectIndex[51] = { nullptr }; 	/* includes name of objects for mouseover + index to object file*/
	bool animationFlag;											/* true if there is a secondary animation */
	Common::String animationName;								/* name of the secondary animation, 8 chars*/
	bool paletteAnimationFlag;									/* true if there exist palette animation */
	uint16 palettePointer;										/* points to the screen palette */
	Common::Point secondaryAnimTrajectory[300];					/* trajectory of the secondary animation */
	uint16 secondaryAnimDirections[300];						/* directions of the secondary trajectory. Pos 300 reflects object code. */
	uint16 secondaryTrajectoryLength;							/* length of the trajectory of the secondary animation */
	~RoomFileRegister() {
		for (int i = 0; i < 51; i++) {
			delete screenObjectIndex[i];
			screenObjectIndex[i] = nullptr;
		}
	}
};

struct SavedGame {
	uint roomCode,
		trajectoryLength,
		currentTrajectoryIndex,
		backpackObjectCode,
		rightSfxVol,
		leftSfxVol,
		musicVolRight,
		musicVolLeft,
		oldGridX,
		oldGridY,
		secAnimDepth,
		secAnimDir,
		secAnimX,
		secAnimY,
		secAnimIFrame;
	byte currentZone,
		targetZone,
		oldTargetZone,
		inventoryPosition,
		actionCode,
		oldActionCode,
		steps,
		doorIndex,
		characterFacingDir,
		iframe,
		gamePart;
	bool isSealRemoved,
		isPottersManualDelivered,
		obtainedList1,
		obtainedList2,
		list1Complete,
		list2Complete,
		isVasePlaced,
		isScytheTaken,
		isTridentTaken,
		isPottersWheelDelivered,
		isMudDelivered,
		isGreenDevilDelivered,
		isCupboardOpen,
		isChestOpen,
		isTVOn,
		isTrapSet,
		isRedDevilCaptured;
	InventoryEntry mobj[kInventoryIconCount];
	int element1,
		element2,
		characterPosX,
		characterPosY,
		xframe2,
		yframe2;
	Common::String oldInventoryObjectName,
		objetomoinventoryObjectNamehila;
	Common::String characterName;
	Route mainRoute;
	uint16 firstList[5], secondList[5];
	Common::Point trajectory[300];
	// Conversation topic unlocks
	bool firstTimeTopicA[kCharacterCount],
		firstTimeTopicB[kCharacterCount],
		firstTimeTopicC[kCharacterCount],
		bookTopic[kCharacterCount],
		mintTopic[kCharacterCount];

	bool caves[5];
	uint niche[2][4];
};

typedef byte Palette[768];

enum HOTKEYS {
    TALK = 0,
    PICKUP = 1,
    LOOKAT = 2,
    USE = 3,
    OPEN = 4,
    CLOSE = 5,
    YES = 6,
    NO = 7
};

} // End of namespace Tot

#endif
