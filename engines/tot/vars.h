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
#ifndef TOT_VARS_H
#define TOT_VARS_H

#include "common/file.h"
#include "common/memstream.h"
#include "common/scummsys.h"

#include "tot/util.h"

namespace Tot {

const int verbRegSize = 263;
const int roomRegSize = 10856;
const int itemRegSize = 279;

const int list1Index = 19;
const int list2Index = 20;
const int list1code = 149;
const int list2code = 150;
/**
 * Num of icons in the inventory
 */
const int inventoryIconCount = 34;
const int numScreenOverlays = 15;
/**
 * Num of depth levels
 */
const int depthLevelCount = 15;
/**
 * Num of frames of the main character in a single direction
 */
const int walkFrameCount = 16;
/**
 * Num of frames in the secondary animation
 */
const int secAnimationFrameCount = 50;
/**
 * X factor of the screen grid
 * This results in 320/8 = 40 quadrants.
 */
const int xGridCount = 8;
/**
 * Y factor of the screen grid
 * This results in 140/5 = 28 quadrants.
 */
const int yGridCount = 5;
/**
 * Y offset of feet within character sprite (for adjustment of path finding)
 */
const int characerCorrectionY = 49;
/**
 * X offset of feet within character sprite (for adjustment of path finding)
 */
const int characterCorrectionX = 14;
const int objectNameLength = 20;
const int characterCount = 9;
/**
 * Size of an inventory icon (39x26)
 */
const int inventoryIconSize = 1018;
/**
 * Number of points in a
 */
const int routePointCount = 7;

/**
 * Trajectory changes
 */
typedef Common::Point route[routePointCount];

struct ObjectInfo {
	uint16 code, posx, posy, posx2, posy2;
};

struct CharacterAnim {
	uint16 depth;
	byte *bitmap[4][walkFrameCount + 30]; // 30 = 3 actions * 10 frames each
};

struct SecondaryAnim {
	uint16 depth, dir, posx, posy;
	byte *bitmap[4][secAnimationFrameCount];
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
	bool 			continued;  // true if the next entry is a continuation of this one
	uint16 			response;   // entry number of reply
	int32 			pointer;
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

	byte xrej1, yrej1, xrej2, yrej2; /* position of patches below*/

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
	int32 bitmapPointer;
	uint16 bitmapSize;
	uint16 coordx, coordy, depth;
};

struct RoomFileRegister {
	uint16 code;
	int32 roomImagePointer;
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
	RoomObjectListEntry *screenObjectIndex[51] = {NULL}; 	/* includes name of objects for mouseover + index to object file*/
	bool animationFlag;										/* true if there is a secondary animation */
	Common::String animationName;							/* name of the secondary animation, 8 chars*/
	bool paletteAnimationFlag;								/* true if there exist palette animation */
	uint16 palettePointer;									/* points to the screen palette */
	Common::Point secondaryAnimTrajectory[300];				/* trajectory of the secondary animation */
	uint16 secondaryAnimDirections[300];					/* directions of the secondary trajectory. Pos 300 reflects object code. */
	uint16 secondaryTrajectoryLength;						/* length of the trajectory of the secondary animation */
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
	InventoryEntry mobj[inventoryIconCount];
	int element1,
		element2,
		characterPosX,
		characterPosY,
		xframe2,
		yframe2;
	Common::String oldInventoryObjectName,
		objetomoinventoryObjectNamehila;
	Common::String characterName;
	route mainRoute;
	uint16 firstList[5], secondList[5];
	Common::Point trajectory[300];
	// Conversation topic unlocks
	bool firstTimeTopicA[characterCount],
		firstTimeTopicB[characterCount],
		firstTimeTopicC[characterCount],
		bookTopic[characterCount],
		mintTopic[characterCount];

	bool caves[5];
	uint niche[2][4];
};

typedef byte palette[768];

extern Common::MemorySeekableReadWriteStream *conversationData;
extern Common::MemorySeekableReadWriteStream *rooms;
extern Common::MemorySeekableReadWriteStream *invItemData;
/**
 * Frame index of the mouse mask
 */
extern byte mouseMaskIndex;
/**
 * Coords of the mouse sprite
 */
extern uint mouseX, mouseY;
/**
 * Coords of mouse clicks
 */
extern uint mouseClickX, mouseClickY;
/**
 * Mouse clicks for both buttons
 */
extern uint npraton2, npraton;

/**
 * Previous positions of the mouse within the screen grid
 */
extern uint oldGridX, oldGridY;

extern SavedGame savedGame;

extern bool isSealRemoved;

/**
 * Flag to enable screen/room change
 */
extern bool roomChange;
extern bool isTVOn,
	isVasePlaced,
	isScytheTaken,
	isTridentTaken,
	isPottersWheelDelivered,
	isMudDelivered,
	isGreenDevilDelivered,
	isRedDevilCaptured,
	isPottersManualDelivered,
	isCupboardOpen,
	isChestOpen,
	isTrapSet,
	isPeterCoughing;

/**
 * Flag for temporary savegame
 */
extern bool inGame;
/**
 * Flag for first time run of the game.
 */
extern bool firstTimeDone;


extern bool isIntroSeen;

/**
 * Flag to exit program.
 */
extern bool shouldQuitGame;
/**
 * Flag to initialize game
 */
extern bool startNewGame;
/**
 * Flag to resume game
 */
extern bool continueGame;
/**
 * Flag to load a game upon start.
 */
extern bool isSavingDisabled;
/**
 * true if sprites should be drawn
 */
extern bool isDrawingEnabled;
/**
 * Flag for secondary animation
 */
extern bool isSecondaryAnimationEnabled;
/**
 * 54 color palette slice.
 */
extern palette palAnimSlice;
/**
 * General palette
 */
extern palette pal;

/**
 * These are the icons currnetly in the inventory
 */
extern InventoryEntry mobj[inventoryIconCount];
/**
 * Keeps an array of all inventory icon bitmaps
 */
extern byte *inventoryIconBitmaps[inventoryIconCount];

/**
 * Delay of palette animation
 */
extern byte palAnimStep;
/**
 * Position within inventory
 */
extern byte inventoryPosition;
/**
 * Currently selected action.
 */
extern byte actionCode;
/**
 * Previously selected action.
 */
extern byte oldActionCode;
/**
 * Number of trajectory changes
 */
extern byte steps;
/**
 * index of currently selected door.
 */
extern byte doorIndex;
/**
 * Aux for palette animation
 */
extern byte isPaletteAnimEnabled;
/**
 * 1 first part, 2 second part
 */
extern byte gamePart;
/**
 * Number of frames of secondary animation
 */
extern byte secondaryAnimationFrameCount;
/**
 * Number of directions of the secondary animation
 */
extern byte secondaryAnimDirCount;
/**
 * Data protection control
 */
extern byte cpCounter, cpCounter2;
/**
 * Coordinates of target step
 */
extern byte destinationStepX, destinationStepY;
/**
 * Current character facing direction
 * 0: upwards
 * 1: right
 * 2: downwards
 * 3: left
 */
extern byte charFacingDirection;

/**
 * Width and height of secondary animation
 */
extern uint secondaryAnimWidth, secondaryAnimHeight;
/**
 * Code of selected object in the backpack
 */
extern uint backpackObjectCode;
/**
 * Foo
 */
extern uint foo;
/**
 * Auxiliary vars for grid update
 */
extern uint oldposx, oldposy;
extern uint rightSfxVol, leftSfxVol;
extern uint musicVolRight, musicVolLeft;

/**
 * Amplitude of movement
 */
extern int element1, element2;
/**
 * Current position of the main character
 */
extern int characterPosX, characterPosY;
/**
 * Target position of the main character?
 */
extern int xframe2, yframe2;
/**
 * Text map
 */
extern Common::File verb;
/**
 * Auxiliary vars with current inventory object name.
 */
extern Common::String oldInventoryObjectName, inventoryObjectName;

extern Common::String photoFileName;
/**
 * Name of player
 */
extern Common::String characterName;

extern Common::String decryptionKey;

extern uint niche[2][4];

extern RoomFileRegister *currentRoomData;

extern ScreenObject regobj;
/**
 * New movement to execute.
 */
extern route mainRoute;
/**
 * Matrix of positions for a trajectory between two points
 */
extern Common::Point trajectory[300];

/**
 * Longitude of the trajectory matrix.
 */
extern uint trajectoryLength;
/**
 * Position within the trajectory matrix
 */
extern uint currentTrajectoryIndex;
/**
 * Position within the trajectory matrix for secondary animation
 */
extern uint currentSecondaryTrajectoryIndex;
/**
 * Screen areas
 */
extern byte currentZone, targetZone, oldTargetZone;
/**
 * Amplitude of grid slices
 */
extern byte maxXGrid, maxYGrid;

/**
 * capture of movement grid of secondary animation
 */
extern byte movementGridForSecondaryAnim[10][10];
/**
 * capture of mouse grid of secondary animation
 */
extern byte mouseGridForSecondaryAnim[10][10];
/**
 * movement mask for grid of secondary animation
 */
extern byte maskGridSecondaryAnim[10][10];

/**
 * mouse mask for grid of secondary animation
 */
extern byte maskMouseSecondaryAnim[10][10];

extern bool list1Complete,
	list2Complete,
	obtainedList1, // whether we've been given list 1
	obtainedList2; // whether we've been given list 2

extern bool firstTimeTopicA[characterCount],
	firstTimeTopicB[characterCount],
	firstTimeTopicC[characterCount],
	bookTopic[characterCount],
	mintTopic[characterCount];

extern bool caves[5];
/**
 * First and second lists of objects to retrieve in the game
 */
extern uint16 firstList[5],
	secondList[5];
/**
 * Animation sequence
 */
extern CharacterAnim mainCharAnimation;
extern uint mainCharFrameSize;
extern SecondaryAnim secondaryAnimation;
extern uint secondaryAnimFrameSize;
/**
 * Max num of loaded frames for secondary animation
 */
extern byte maxSecondaryAnimationFrames;
/**
 * Index of fade effect for room change
 */
extern byte transitionEffect;
/**
 * Frame number for the animations
 */
extern byte iframe, iframe2;

extern long screenSize;

/**
 * Depth of screenobjects
 */
extern ObjectInfo depthMap[numScreenOverlays];
/**
 * Bitmaps of screenobjects
 */
extern byte *screenLayers[numScreenOverlays];
/**
 * Current frame of main character
 */
extern byte *curCharacterAnimationFrame;
/**
 * Current frame of secondary animation
 */
extern byte *curSecondaryAnimationFrame;

/**
 * Pointer storing the screen as it displays on the game
 */
extern byte *sceneBackground;

/**
 * Dirty patch of screen to repaint on every frame
 */
extern byte *characterDirtyRect;
/**
 * Stores a copy of the background bitmap
 */
extern byte *backgroundCopy;

extern uint currentRoomNumber;

extern bool isLoadingFromLauncher;

extern bool saveAllowed;

void initializeScreenFile();
void clearObj();
void clearScreenData();
void initPlayAnim();
void resetGameState();
void clearVars();

} // End of namespace Tot
#endif
