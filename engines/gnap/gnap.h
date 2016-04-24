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

#ifndef GNAP_H
#define GNAP_H

#include "common/array.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/winexe.h"
#include "common/winexe_pe.h"
#include "engines/engine.h"
#include "graphics/pixelformat.h"
#include "graphics/wincursor.h"

#include "gnap/debugger.h"
#include "gnap/resource.h"
#include "gnap/scenes/scenecore.h"

struct ADGameDescription;

namespace Gnap {

class DatManager;
class SequenceResource;
class SpriteResource;
class GameSys;
class SoundMan;

#define GNAP_SAVEGAME_VERSION 1

struct MouseButtonState {
	bool _left;
	bool _right;
	MouseButtonState() : _left(false), _right(false) {
	}
};

struct Hotspot {
	int16 _x1, _y1, _x2, _y2;
	uint16 _flags;
	int _id;
	bool isPointInside(int16 x, int16 y) const {
		return x >= _x1 && x <= _x2 && y >= _y1 && y <= _y2;
	}
	bool isFlag(uint16 flag) const {
		return (_flags & flag) != 0;
	}
};

struct GridStruct {
	int _deltaX, _deltaY;
	int _gridX1, _gridY1;
	int _sequenceId;
	int _id;
};

const int kMaxGridStructs = 30;
const int kMaxTimers = 10;

enum GnapDebugChannels {
	kDebugBasic	= 1 << 0
};

enum {
	SF_NONE             = 0x0000,
	SF_LOOK_CURSOR		= 0x0001,
	SF_GRAB_CURSOR		= 0x0002,
	SF_TALK_CURSOR		= 0x0004,
	SF_PLAT_CURSOR		= 0x0008,
	SF_DISABLED			= 0x0010,
	SF_WALKABLE			= 0x0020,
	SF_EXIT_L_CURSOR	= 0x0040,
	SF_EXIT_R_CURSOR	= 0x0080,
	SF_EXIT_U_CURSOR	= 0x0100,
	SF_EXIT_D_CURSOR	= 0x0200,
	SF_EXIT_NW_CURSOR	= 0x0400,
	SF_EXIT_NE_CURSOR	= 0x0800,
	SF_EXIT_SW_CURSOR	= 0x1000,
	SF_EXIT_SE_CURSOR	= 0x2000
};

enum {
	LOOK_CURSOR		= 0,
	GRAB_CURSOR		= 1,
	TALK_CURSOR		= 2,
	PLAT_CURSOR		= 3,
	NOLOOK_CURSOR	= 4,
	NOGRAB_CURSOR	= 5,
	NOTALK_CURSOR	= 6,
	NOPLAT_CURSOR	= 7,
	EXIT_L_CURSOR	= 8,
	EXIT_R_CURSOR	= 9,
	EXIT_U_CURSOR	= 10,
	EXIT_D_CURSOR	= 11,
	EXIT_NE_CURSOR	= 12,
	EXIT_NW_CURSOR	= 13,
	EXIT_SE_CURSOR	= 14,
	EXIT_SW_CURSOR	= 15,
	WAIT_CURSOR		= 16
};

enum {
	gskPullOutDevice			= 0,
	gskPullOutDeviceNonWorking	= 1,
	gskIdle						= 2,
	gskBrainPulsating			= 3,
	gskImpossible				= 4,
	gskScratchingHead			= 5,
	gskDeflect					= 6,
	gskUseDevice				= 7,
	gskMoan1					= 8,
	gskMoan2					= 9
};

enum {
	kItemMagazine			= 0,
	kItemMud				= 1,
	kItemGrass				= 2,
	kItemDisguise			= 3,
	kItemNeedle				= 4,
	kItemTwig				= 5,
	kItemGas				= 6,
	kItemKeys				= 7,
	kItemDice				= 8,
	kItemTongs				= 9,
	kItemQuarter			= 10,
	kItemQuarterWithHole	= 11,
	kItemDiceQuarterHole	= 12,
	kItemWrench				= 13,
	kItemCowboyHat			= 14,
	kItemGroceryStoreHat	= 15,
	kItemBanana				= 16,
	kItemTickets			= 17,
	kItemPicture			= 18,
	kItemEmptyBucket		= 19,
	kItemBucketWithBeer		= 20,
	kItemBucketWithPill		= 21,
	kItemPill				= 22,
	kItemHorn				= 23,
	kItemJoint				= 24,
	kItemChickenBucket		= 25,
	kItemGum				= 26,
	kItemSpring				= 27,
	kItemLightbulb			= 28,
	kItemCereals			= 29
};

enum {
	kGFPlatypus				= 0,
	kGFMudTaken				= 1,
	kGFNeedleTaken			= 2,
	kGFTwigTaken			= 3,
	kGFUnk04				= 4,
	kGFKeysTaken			= 5,
	kGFGrassTaken			= 6,
	kGFBarnPadlockOpen		= 7,
	kGFTruckFilledWithGas	= 8,
	kGFTruckKeysUsed		= 9,
	kGFPlatyPussDisguised	= 10,
	kGFSceneFlag1			= 11,
	kGFGnapControlsToyUFO	= 12,
	kGFUnk13				= 13, // Tongue Fight Won?
	kGFUnk14				= 14,
	kGFSpringTaken			= 15,
	kGFUnk16				= 16,
	kGFJointTaken			= 17,
	kGFUnk18				= 18,
	kGFGroceryStoreHatTaken	= 19,
	kGFPictureTaken			= 20,
	kGFUnk21				= 21,
	kGFUnk22				= 22,
	kGFUnk23				= 23,
	kGFUnk24				= 24,
	kGFUnk25				= 25,
	kGFPlatypusTalkingToAssistant = 26,
	kGFUnk27				= 27,
	kGFUnk28				= 28,
	kGFGasTaken				= 29,
	kGFUnk30				= 30,
	kGFUnk31				= 31
};

/*
	Game flags
		0	1			With Platypus
		1	2			Mud taken
		2	4			Needle taken
		3	8			Twig taken
		4	0x10
		5	0x20		Keys taken
		6	0x40		Grass taken
		7	0x80		Barn padlock open
		8	0x100		Truck filled with gas
		9	0x200		Truck keys used
		10	0x400		Platypus disguised
		11	0x800		Scene flag 1
		12	0x1000		Gnap controls toy ufo
		13	0x2000
		14	0x4000
		15	0x8000		Spring taken
		16	0x10000
		17	0x20000		Joint taken
		18	0x40000
		19	0x80000		Grocery store hat taken
		20	0x100000	Picture taken
		21	0x200000
		22	0x400000
		23	0x800000
		24	0x1000000
		25	0x2000000
		26	0x4000000	Platypus talking with toy shop assistant
		27	0x8000000
		28	0x10000000
		29	0x20000000	Gas taken
		30	0x40000000
		31	0x80000000


*/

	// 0x7AB	Gnap walks to the right 1
	// 0x7AC	Gnap walks to the right 2
	// 0x7AF	Gnap walks to the left 1
	// 0x7B0	Gnap walks to the left 2

struct Scene51Item {
	int _currSequenceId;
	int _droppedSequenceId;
	int _x, _y;
	int _collisionX;
	bool _canCatch;
	bool _isCollision;
	int _x2;
	int _id;
};

enum Facing {
	kDirNone = 0,
	kDirBottomRight = 1,
	kDirBottomLeft = 3,
	kDirUnk4 = 4,
	kDirUpLeft = 5,
	kDirUpRight = 7
};

struct GnapSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
};

class GnapEngine : public Engine {
protected:
	Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	GnapEngine(OSystem *syst, const ADGameDescription *gd);
	~GnapEngine();
private:
	const ADGameDescription *_gameDescription;
	Graphics::PixelFormat _pixelFormat;
	int _loadGameSlot;

public:	
	Common::RandomSource *_random;
	
	Common::PEResources *_exe;
	
	DatManager *_dat;
	SpriteCache *_spriteCache;
	SoundCache *_soundCache;
	SequenceCache *_sequenceCache;
	GameSys *_gameSys;
	SoundMan *_soundMan;
	Debugger *_debugger;
	Scene *_scene;

	int _lastUpdateClock;
	
	int _debugLevel;
	bool _gameDone;
	
	byte _keyPressState[512];
	byte _keyDownState[512];
	
	bool _isPaused;
	Graphics::Surface *_pauseSprite;
	int _timers[kMaxTimers], _savedTimers[kMaxTimers];
	
	MouseButtonState _mouseButtonState;
	MouseButtonState _mouseClickState;
	
	uint32 _keyStatus1[2];
	
	bool _sceneSavegameLoaded, _wasSavegameLoaded;
	
	Graphics::Surface *_backgroundSurface;
	int _prevSceneNum, _currentSceneNum, _newSceneNum;	
	bool _sceneDone, _sceneWaiting;

	uint32 _inventory, _gameFlags;
	
	Hotspot _hotspots[20];
	Common::Point _hotspotsWalkPos[20];
	int _hotspotsCount;
	int _sceneClickedHotspot;
	
	bool _isWaiting;
	bool _isLeavingScene;
	
	bool _isStockDatLoaded;

	int _newCursorValue, _cursorValue;

	int _verbCursor, _cursorIndex;
	int _mouseX, _mouseY;
	int _leftClickMouseX, _leftClickMouseY;
	
	Graphics::Surface *_grabCursorSprite;
	int _currGrabCursorX, _currGrabCursorY;
	int _grabCursorSpriteIndex, _newGrabCursorSpriteIndex;
	
	Graphics::Surface *_fullScreenSprite;
	int _fullScreenSpriteId;
	
	int _deviceX1, _deviceY1, _deviceX2, _deviceY2;
	
	int _soundTimerIndexA;
	int _soundTimerIndexB;
	int _soundTimerIndexC;
	int _idleTimerIndex;
	
	void updateEvents();
	void gameUpdateTick();
	void saveTimers();
	void restoreTimers();
	
	void pauseGame();
	void resumeGame();
	void updatePause();
	
	int getRandom(int max);
	
	int readSavegameDescription(int savegameNum, Common::String &description);
	int loadSavegame(int savegameNum);
	Common::Error saveGameState(int slot, const Common::String &desc);
	Common::Error loadGameState(int slot);
	Common::String generateSaveName(int slot);
	void synchronize(Common::Serializer &s);
	void writeSavegameHeader(Common::OutSaveFile *out, GnapSavegameHeader &header);
	static bool readSavegameHeader(Common::InSaveFile *in, GnapSavegameHeader &header);
	
	void delayTicks(int a1);
	void delayTicksCursor(int a1);
	
	void setHotspot(int index, int16 x1, int16 y1, int16 x2, int16 y2, uint16 flags = 0,
		int16 walkX = -1, int16 walkY = -1);
	int getHotspotIndexAtPos(int16 x, int16 y);
	void updateCursorByHotspot();
	int getClickedHotspotId();
	
	int getInventoryItemSpriteNum(int index);
	
	void updateMouseCursor();
	void setVerbCursor(int verbCursor);
	void setCursor(int cursorIndex);
	void showCursor();
	void hideCursor();
	
	void setGrabCursorSprite(int index);
	void createGrabCursorSprite(int spriteId);
	void freeGrabCursorSprite();
	void updateGrabCursorSprite(int x, int y);
	
	void invClear();
	void invAdd(int itemId);
	void invRemove(int itemId);
	bool invHas(int itemId);
	
	void clearFlags();
	void setFlag(int num);
	void clearFlag(int num);
	bool isFlag(int num);
	
	Graphics::Surface *addFullScreenSprite(int resourceId, int id);
	void removeFullScreenSprite();
	void showFullScreenSprite(int resourceId);
	
	void queueInsertDeviceIcon();
	void insertDeviceIconActive();
	void removeDeviceIconActive();
	void setDeviceHotspot(int hotspotIndex, int x1, int y1, int x2, int y2);
	
	int getSequenceTotalDuration(int resourceId);
	
	bool isSoundPlaying(int resourceId);
	void playSound(int resourceId, bool looping);
	void stopSound(int resourceId);
	void setSoundVolume(int resourceId, int volume);

	void updateTimers();
	
	void initGameFlags(int num);
	void loadStockDat();

	void mainLoop();
	void initScene();
	void endSceneInit();
	void afterScene();
	
	int initSceneLogic();
	void runSceneLogic();
	
	void checkGameKeys();
	
	void startSoundTimerA(int timerIndex);
	int playSoundA();
	void startSoundTimerB(int timerIndex);
	int playSoundB();
	void startSoundTimerC(int timerIndex);
	int playSoundC();
	void startIdleTimer(int timerIndex);
	void updateIdleTimer();
	
	void screenEffect(int dir, byte r, byte g, byte b);
	
	bool isKeyStatus1(int key);
	bool isKeyStatus2(int key);
	void clearKeyStatus1(int key);
	void clearAllKeyStatus1();
	
	void deleteSurface(Graphics::Surface **surface);
	
	// Menu
	int _menuStatus;
	int _menuSpritesIndex;
	bool _menuDone;
	Graphics::Surface *_menuBackgroundSurface;
	Graphics::Surface *_menuQuitQuerySprite;
	Graphics::Surface *_largeSprite;
	Graphics::Surface *_menuSaveLoadSprite;
	Graphics::Surface *_menuSprite2;
	Graphics::Surface *_menuSprite1;	
	char _savegameFilenames[7][30];
	Graphics::Surface *_savegameSprites[7];
	Graphics::Surface *_spriteHandle;
	Graphics::Surface *_cursorSprite;
	int _menuInventoryIndices[30];
	Graphics::Surface *_menuInventorySprites[30];
	int _savegameIndex;
	void createMenuSprite();
	void freeMenuSprite();
	void initMenuHotspots1();
	void initMenuHotspots2();
	void initMenuQuitQueryHotspots();
	void initSaveLoadHotspots();
	void drawInventoryFrames();
	void insertInventorySprites();
	void removeInventorySprites();	
	void runMenu();
	void updateMenuStatusInventory();
	void updateMenuStatusMainMenu();
	void updateMenuStatusSaveGame();
	void updateMenuStatusLoadGame();
	void updateMenuStatusQueryQuit();
	
	// Grid common
	int _gnapGridX, _gnapGridY;
	int _platGridX, _platGridY;
	int _gridMinX, _gridMinY;
	int _gridMaxX, _gridMaxY;
	bool isPointBlocked(int gridX, int gridY);
	void initSceneGrid(int gridMinX, int gridMinY, int gridMaxX, int gridMaxY);
	bool testWalk(int animationIndex, int someStatus, int gridX1, int gridY1, int gridX2, int gridY2);

	// Gnap walking
	int _gnapWalkNodesCount;
	GridStruct _gnapWalkNodes[kMaxGridStructs];
	int _gnapX, _gnapY;
	int _gnapWalkDestX, _gnapWalkDestY;
	int _gnapWalkDeltaX, _gnapWalkDeltaY, _gnapWalkDirX, _gnapWalkDirY, _gnapWalkDirXIncr, _gnapWalkDirYIncr;

	int getGnapWalkSequenceId(int deltaX, int deltaY);
	int getGnapWalkStopSequenceId(int deltaX, int deltaY);
	Facing getGnapWalkFacing(int deltaX, int deltaY);
	bool gridSub41F08B(int gridX, int gridY);
	bool gridSub41F5FC(int gridX, int gridY, int index);
	bool gridSub41FAD5(int gridX, int gridY, int index);
	bool gnapFindPath3(int gridX, int gridY);
	bool gnapWalkTo(int gridX, int gridY, int animationIndex, int sequenceId, int flags);
	void gnapWalkStep();
	
	// Platypus walking
	int _platWalkNodesCount;
	GridStruct _platWalkNodes[kMaxGridStructs];
	int _platX, _platY;
	int _platWalkDestX, _platWalkDestY;
	int _platWalkDeltaX, _platWalkDeltaY, _platWalkDirX, _platWalkDirY, _platWalkDirXIncr, _platWalkDirYIncr;
	
	int getPlatypusWalkSequenceId(int deltaX, int deltaY);
	bool gridSub423750(int gridX, int gridY);
	bool gridSub423CC1(int gridX, int gridY, int index);
	bool gridSub42419A(int gridX, int gridY, int index);
	bool platFindPath3(int gridX, int gridY);
	bool platypusWalkTo(int gridX, int gridY, int animationIndex, int sequenceId, int flags);
	void platypusWalkStep();
	void platypusMakeRoom();

	// Gnap
	Facing _gnapIdleFacing;
	int _gnapActionStatus;
	int _gnapBrainPulseNum, _gnapBrainPulseRndValue;
	int _gnapSequenceId, _gnapSequenceDatNum, _gnapId;

	int getGnapSequenceId(int kind, int gridX, int gridY);
	int getGnapShowSequenceId(int index, int gridX, int gridY);
	void gnapIdle();
	void gnapActionIdle(int sequenceId);
	void playGnapSequence(int sequenceId);
	void playGnapImpossible(int gridX, int gridY);
	void playGnapScratchingHead(int gridX, int gridY);
	void playGnapMoan1(int gridX, int gridY);
	void playGnapMoan2(int gridX, int gridY);
	void playGnapBrainPulsating(int gridX, int gridY);
	void playGnapPullOutDevice(int gridX, int gridY);
	void playGnapPullOutDeviceNonWorking(int gridX, int gridY);
	void playGnapUseDevice(int gridX, int gridY);
	void playGnapIdle(int gridX, int gridY);
	void playGnapShowItem(int itemIndex, int gridLookX, int gridLookY);
	void playGnapShowCurrItem(int gridX, int gridY, int gridLookX, int gridLookY);
	void updateGnapIdleSequence();
	void updateGnapIdleSequence2();
	void initGnapPos(int gridX, int gridY, Facing facing);
	void gnapInitBrainPulseRndValue();
	void gnapUseDeviceOnPlatypuss();
	void doCallback(int callback);
	bool gnapPlatypusAction(int gridX, int gridY, int platSequenceId, int callback);
	void gnapKissPlatypus(int callback);
	void gnapUseJointOnPlatypus();
	void gnapUseDisguiseOnPlatypus();
	
	// Platypus
	Facing _platypusFacing;
	int _platypusActionStatus;
	int _platypusSequenceId, _platypusSequenceDatNum, _platypusId;
	int getPlatypusSequenceId();
	void playPlatypusSequence(int sequenceId);
	void updatePlatypusIdleSequence();
	void platypusSub426234();
	void initPlatypusPos(int gridX, int gridY, Facing facing);
	
	// Scenes
	
	int _toyUfoNextSequenceId, _toyUfoSequenceId;
	int _toyUfoId;
	int _toyUfoActionStatus;
	int _toyUfoX;
	int _toyUfoY;
	
	void initGlobalSceneVars();

    bool sceneXX_sub_4466B1();
	void sceneXX_playRandomSound(int timerIndex);
	void playSequences(int fullScreenSpriteId, int sequenceId1, int sequenceId2, int sequenceId3);

	// Shared by scenes 17 & 18
	int _s18_garbageCanPos;

	// Scene 4x
	void toyUfoSetStatus(int flagNum);
	int toyUfoGetSequenceId();
	bool toyUfoCheckTimer();
	void toyUfoFlyTo(int destX, int destY, int minX, int maxX, int minY, int maxY, int animationIndex);

	// Scene 51
	int _s51_cashAmount;
	int _s51_digits[4];
	int _s51_digitSequenceIds[4];
	int _s51_guySequenceId, _s51_guyNextSequenceId;
	int _s51_itemsCaughtCtr;
	int _s51_dropSpeedTicks;
	int _s51_nextDropItemKind, _s51_itemInsertX, _s51_itemInsertDirection;
	int _s51_platypusSequenceId, _s51_platypusNextSequenceId, _s51_platypusJumpSequenceId;
	bool _s51_dropLoseCash;
	int _s51_itemsCtr,_s51_itemsCtr1, _s51_itemsCtr2;
	Scene51Item _s51_items[6];

	int scene51_init();
	void scene51_updateHotspots();
	void scene51_clearItem(Scene51Item *item);
	void scene51_dropNextItem();
	void scene51_updateItemAnimations();
	int scene51_checkCollision(int sequenceId);
	void scene51_updateItemAnimation(Scene51Item *item, int index);
	void scene51_removeCollidedItems();
	int scene51_itemIsCaught(Scene51Item *item);
	bool scene51_isJumpingRight(int sequenceId);
	bool scene51_isJumpingLeft(int sequenceId);
	bool scene51_isJumping(int sequenceId);
	void scene51_waitForAnim(int animationIndex);
	int scene51_getPosRight(int sequenceId);
	int scene51_getPosLeft(int sequenceId);
	void scene51_playIntroAnim();
	void scene51_updateGuyAnimation();
	int scene51_incCashAmount(int sequenceId);
	void scene51_winMinigame();
	void scene51_playCashAppearAnim();
	void scene51_updateCash(int amount);
	void scene51_drawDigit(int digit, int position);
	void scene51_initCashDisplay();
	void scene51_run();
	
	// Scene 52
	int _s52_liveAlienRows;
	int _s52_gameScore;
	bool _s52_soundToggle;
	int _s52_arcadeScreenLeft;
	int _s52_arcadeScreenRight;
	int _s52_arcadeScreenBottom;
	int _s52_shipsLeft;
	int _s52_shieldSpriteIds[3];
	int _s52_shieldPosX[3];
	int _s52_shipPosX;
	int _s52_shipCannonPosX, _s52_shipCannonPosY;
	bool _s52_shipCannonFiring;
	bool _s52_shipCannonFired; 
	int _s52_shipCannonWidth, _s52_shipCannonHeight;
	int _s52_shipCannonTopY;
	int _s52_shipMidX, _s52_shipMidY;
	bool _s52_shipFlag;
	bool _s52_aliensInitialized;
	int _s52_alienSpeed, _s52_alienDirection;
	int _s52_alienWidth, _s52_alienHeight;
	int _s52_alienLeftX, _s52_alienTopY;
	int _s52_alienRowDownCtr;
	int _s52_alienRowKind[7];
	int _s52_alienRowAnims[7];
	int _s52_alienRowIds[7];
	int _s52_alienRowXOfs[7];
	int _s52_alienCannonFired[3];
	int _s52_alienCannonPosX[3];
	int _s52_alienCannonPosY[3];
	int _s52_alienCannonSequenceIds[3];
	int _s52_alienCannonIds[3];
	bool _s52_alienWave, _s52_alienSingle;
	int _s52_alienCounter;
	bool _s52_bottomAlienFlag;
	int _s52_aliensCount;
	int _s52_items[8][5];
	int _s52_nextUfoSequenceId, _s52_ufoSequenceId;
	int scene52_init();
	void scene52_updateHotspots();
	void scene52_update();
	void scene52_initShipCannon(int bottomY);
	void scene52_initAlienCannons();
	void scene52_fireShipCannon(int posX);
	void scene52_fireAlienCannon();
	int scene52_getFreeShipCannon();
	int scene52_getFreeAlienCannon();
	void scene52_updateShipCannon();
	void scene52_updateAlienCannons();
	void scene52_initAliens();
	void scene52_initAlienRowKind(int rowNum, int alienKind);
	void scene52_insertAlienRow(int rowNum);
	void scene52_insertAlienRowAliens(int rowNum);
	void scene52_updateAlienRow(int rowNum);
	void scene52_moveDownAlienRow();
	int scene52_updateHitAlien();
	int scene52_getHitAlienNum(int rowNum);
	int scene52_alienCannonHitShip(int cannonNum);
	int scene52_alienCannonHitShield(int cannonNum);
	bool scene52_shipCannonHitShield(int cannonNum);
	bool scene52_shipCannonHitAlien();
	void scene52_shipExplode();
	bool scene52_checkAlienRow(int rowNum);
	void scene52_updateAlienRowXOfs();
	void scene52_initAlienSize();
	void scene52_playSound();
	void scene52_updateAliens();
	void scene52_updateAlien(int rowNum);
	void scene52_loseShip();
	void scene52_initShields();
	void scene52_initAnims();
	void scene52_drawScore(int score);
	void scene52_run();
	
	// Scene 53
	int _s53_currHandSequenceId;
	bool _s53_isGnapPhoning;
	int _s53_callsMadeCtr;
	uint _s53_callsRndUsed;
	int scene53_init();
	void scene53_updateHotspots();
	int scene53_pressPhoneNumberButton(int phoneNumber, int buttonNum);
	int scene53_getRandomCallIndex();
	void scene53_runRandomCall();
	void scene53_runChitChatLine();
	void scene53_run();

	// Scene 54
	void scene54_initCutscene1();
	void scene54_initCutscene2();
};

} // End of namespace Gnap

#endif // GNAP_H
