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
#include "common/str.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/winexe.h"
#include "common/winexe_pe.h"
#include "engines/engine.h"
#include "graphics/pixelformat.h"
#include "graphics/wincursor.h"

#include "gnap/resource.h"

struct ADGameDescription;

namespace Gnap {

class DatManager;
class SequenceResource;
class SpriteResource;
class GameSys;
class SoundMan;

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
	int deltaX, deltaY;
	int gridX1, gridY1;
	int sequenceId;
	int id;
};

const int kMaxGridStructs = 30;
const int kMaxTimers = 10;

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
	
	
	kGFEnd
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

struct Scene49Obstacle {
	int currSequenceId;
	int closerSequenceId;
	int passedSequenceId;
	int splashSequenceId;
	int collisionSequenceId;
	int prevId;
	int currId;
	int laneNum;
};

struct Scene51Item {
	int currSequenceId;
	int droppedSequenceId;
	int x, y;
	int collisionX;
	int canCatch;
	int isCollision;
	int x2;
	int id;
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
public:	
	Common::RandomSource *_random;
	
	Common::PEResources *_exe;
	
	DatManager *_dat;
	SpriteCache *_spriteCache;
	SoundCache *_soundCache;
	SequenceCache *_sequenceCache;
	GameSys *_gameSys;
	SoundMan *_soundMan;
	
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
	int getGnapWalkFacing(int deltaX, int deltaY);
	bool gridSub41F08B(int gridX, int gridY);
	bool gridSub41F5FC(int gridX, int gridY, int index);
	bool gridSub41FAD5(int gridX, int gridY, int index);
	bool gnapFindPath3(int gridX, int gridY);
	bool gnapWalkTo(int gridX, int gridY, int animationIndex, int sequenceId, int flags);
	void gnapWalkStep();
	
	// Beaver walking
	int _platWalkNodesCount;
	GridStruct _platWalkNodes[kMaxGridStructs];
	int _platX, _platY;
	int _platWalkDestX, _platWalkDestY;
	int _platWalkDeltaX, _platWalkDeltaY, _platWalkDirX, _platWalkDirY, _platWalkDirXIncr, _platWalkDirYIncr;
	
	int getBeaverWalkSequenceId(int deltaX, int deltaY);
	bool gridSub423750(int gridX, int gridY);
	bool gridSub423CC1(int gridX, int gridY, int index);
	bool gridSub42419A(int gridX, int gridY, int index);
	bool platFindPath3(int gridX, int gridY);
	bool platypusWalkTo(int gridX, int gridY, int animationIndex, int sequenceId, int flags);
	void platypusWalkStep();
	void beaverMakeRoom();

	// Gnap
	int _gnapIdleFacing;
	int _gnapActionStatus;
	int _gnapBrainPulseNum, _gnapBrainPulseRndValue;
	int _gnapSequenceId, _gnapSequenceDatNum, _gnapId;
	int _gnapRandomValue;

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
	void initGnapPos(int gridX, int gridY, int facing);
	void gnapInitBrainPulseRndValue();
	void gnapUseDeviceOnBeaver();
	void doCallback(int callback);
	bool gnapPlatypusAction(int gridX, int gridY, int platSequenceId, int callback);
	void gnapKissPlatypus(int callback);
	void gnapUseJointOnPlatypus();
	void gnapUseDisguiseOnPlatypus();
	
	// Beaver
	int _beaverFacing;
	int _beaverActionStatus;
	int _beaverSequenceId, _beaverSequenceDatNum, _beaverId;
	int getBeaverSequenceId(int kind, int gridX, int gridY);
	void playBeaverSequence(int sequenceId);
	void updateBeaverIdleSequence();
	void beaverSub426234();
	void initBeaverPos(int gridX, int gridY, int facing);
	
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
	
	// Scene 0
	int scene00_init();
	
	// Scene 1
	int _s01_pigsIdCtr, _s01_smokeIdCtr;
	Graphics::Surface *_s01_spaceshipSurface;
	int scene01_init();
	void scene01_updateHotspots();
	void scene01_run();
	void scene01_updateAnimations();
	
	// Scene 2
	int _s02_truckGrillCtr;
	int _s02_nextChickenSequenceId, _s02_currChickenSequenceId;
	int _s02_gnapTruckSequenceId;
	int scene02_init();
	void scene02_updateHotspots();
	void scene02_run();
	void scene02_updateAnimations();
	
	// Scene 3
	bool _s03_platypusHypnotized;	
	bool _s03_platypusScared;
	int _s03_nextPlatSequenceId;
	int _s03_nextFrogSequenceId, _s03_currFrogSequenceId;
	int scene03_init();
	void scene03_updateHotspots();
	void scene03_run();
	void scene03_updateAnimations();

	// Scene 4
	bool _s04_triedWindow;
	int _s04_dogIdCtr;
	int _s04_nextDogSequenceId, _s04_currDogSequenceId;
	int scene04_init();
	void scene04_updateHotspots();
	void scene04_run();
	void scene04_updateAnimations();

	// Scene 5
	int _s05_nextChickenSequenceId, _s05_currChickenSequenceId;
	int scene05_init();
	void scene05_updateHotspots();
	void scene05_run();
	void scene05_updateAnimations();

	// Scene 6
	bool _s06_horseTurnedBack;
	int _s06_nextPlatSequenceId;
	int _s06_nextHorseSequenceId, _s06_currHorseSequenceId;
	int scene06_init();
	void scene06_updateHotspots();
	void scene06_run();
	void scene06_updateAnimations();

	// Scene 7
	int scene07_init();
	void scene07_updateHotspots();
	void scene07_run();
	void scene07_updateAnimations();

	// Scene 8
	int _s08_nextDogSequenceId, _s08_currDogSequenceId;
	int _s08_nextManSequenceId, _s08_currManSequenceId;
	int scene08_init();
	void scene08_updateHotspots();
	void scene08_updateAnimationsCb();
	void scene08_run();
	void scene08_updateAnimations();

	// Scene 9
	int scene09_init();
	void scene09_updateHotspots();
	void scene09_run();
	void scene09_updateAnimations();

	// Scene 10
	int _s10_nextCookSequenceId, _s10_currCookSequenceId;
	int scene10_init();
	void scene10_updateHotspots();
	void scene10_run();
	void scene10_updateAnimations();
	void scene10_updateAnimationsCb();

	// Scene 11
	int _s11_billardBallCtr;
	int _s11_nextHookGuySequenceId, _s11_currHookGuySequenceId;
	int _s11_nextGoggleGuySequenceId, _s11_currGoggleGuySequenceId;
	int scene11_init();
	void scene11_updateHotspots();
	void scene11_run();
	void scene11_updateAnimations();

	// Scene 12
	int _s12_nextBeardGuySequenceId, _s12_currBeardGuySequenceId;
	int _s12_nextToothGuySequenceId, _s12_currToothGuySequenceId;
	int _s12_nextBarkeeperSequenceId, _s12_currBarkeeperSequenceId;
	int scene12_init();
	void scene12_updateHotspots();
	void scene12_run();
	void scene12_updateAnimations();

	// Scene 13
	int _s13_backToiletCtr;
	int scene13_init();
	void scene13_updateHotspots();
	void scene13_showScribble();
	void scene13_run();
	void scene13_updateAnimations();

	// Scene 14
	int scene14_init();
	void scene14_updateHotspots();
	void scene14_run();
	void scene14_updateAnimations();

	// Scene 15
	int _s15_nextRecordSequenceId, _s15_currRecordSequenceId;
	int _s15_nextSlotSequenceId, _s15_currSlotSequenceId;
	int _s15_nextUpperButtonSequenceId, _s15_currUpperButtonSequenceId;
	int _s15_nextLowerButtonSequenceId, _s15_currLowerButtonSequenceId;	
	int scene15_init();
	void scene15_updateHotspots();
	void scene15_run();
	void scene15_updateAnimations();

	// Scene 16
	void scene16_initCutscene();

	// Scene 17
	bool _s17_canTryGetWrench;
	int _s17_wrenchCtr, _s17_platPhoneCtr, _s17_platTryGetWrenchCtr;
	int _s17_nextPhoneSequenceId, _s17_currPhoneSequenceId;
	int _s17_nextWrenchSequenceId, _s17_currWrenchSequenceId;
	int _s17_nextCarWindowSequenceId, _s17_currCarWindowSequenceId;
	int scene17_init();
	void scene17_updateHotspots();
	void scene17_update();
	void scene17_platHangUpPhone();
	void scene17_run();
	void scene17_updateAnimations();

	// Scene 18
	int _s18_garbageCanPos;
	int _s18_platPhoneCtr;
	int _s18_platPhoneIter;
	int _s18_nextPhoneSequenceId, _s18_currPhoneSequenceId;
	Graphics::Surface *_s18_cowboyHatSurface;
	int scene18_init();
	void scene18_updateHotspots();
	void scene18_gnapCarryGarbageCanTo(int x, int y, int animationIndex, int argC, int a5);
	void scene18_putDownGarbageCan(int animationIndex);
	void scene18_platEndPhoning(bool platFl);
	void scene18_closeHydrantValve();
	void scene18_waitForGnapAction();
	void scene18_run();
	void scene18_updateAnimations();

	// Scene 19
	int _s19_currShopAssistantSequenceId, _s19_nextShopAssistantSequenceId;
	int _s19_toyGrabCtr;
	int _s19_shopAssistantCtr;
	Graphics::Surface *_s19_pictureSurface;
	int scene19_init();
	void scene19_updateHotspots();
	void scene19_run();
	void scene19_updateAnimations();

	// Scene 20
	int _s20_currStonerGuySequenceId, _s20_nextStonerGuySequenceId;
	int _s20_currGroceryStoreGuySequenceId, _s20_nextGroceryStoreGuySequenceId;
	int _s20_stonerGuyCtr;
	bool _s20_stonerGuyShowingJoint;
	int _s20_groceryStoreGuyCtr;
	int scene20_init();
	void scene20_updateHotspots();
	void scene20_updateAnimationsCb();
	void scene20_stopSounds();
	void scene20_run();
	void scene20_updateAnimations();

	// Scene 21
	int _s21_currOldLadySequenceId, _s21_nextOldLadySequenceId;
	int scene21_init();
	void scene21_updateHotspots();
	void scene21_run();
	void scene21_updateAnimations();

	// Scene 22
	int _s22_currCashierSequenceId, _s22_nextCashierSequenceId;
	bool _s22_caughtBefore;
	int _s22_cashierCtr;
	int scene22_init();
	void scene22_updateHotspots();
	void scene22_run();
	void scene22_updateAnimations();

	// Scene 23
	int _s23_currStoreClerkSequenceId, _s23_nextStoreClerkSequenceId;
	int scene23_init();
	void scene23_updateHotspots();
	void scene23_run();
	void scene23_updateAnimations();

	// Scene 24
	int _s24_currWomanSequenceId, _s24_nextWomanSequenceId;
	int _s24_boySequenceId;
	int _s24_girlSequenceId;
	int scene24_init();
	void scene24_updateHotspots();
	void scene24_run();
	void scene24_updateAnimations();

	// Scene 25
	int _s25_currTicketVendorSequenceId, _s25_nextTicketVendorSequenceId;
	int scene25_init();
	void scene25_updateHotspots();
	void scene25_playAnims(int index);
	void scene25_run();
	void scene25_updateAnimations();

	// Scene 26
	int _s26_currKidSequenceId, _s26_nextKidSequenceId;
	int scene26_init();
	void scene26_updateHotspots();
	void scene26_run();
	void scene26_updateAnimations();

	// Scene 27
	int _s27_nextJanitorSequenceId;
	int _s27_currJanitorSequenceId;
	int scene27_init();
	void scene27_updateHotspots();
	void scene27_run();
	void scene27_updateAnimations();
		
	// Scene 28
	int _s28_currClownSequenceId, _s28_nextClownSequenceId;
	int _s28_clownTalkCtr;
	int scene28_init();
	void scene28_updateHotspots();
	void scene28_run();
	void scene28_updateAnimations();
	
	// Scene 29
	int _s28_currMonkeySequenceId, _s28_nextMonkeySequenceId;
	int _s28_currManSequenceId, _s28_nextManSequenceId;
	int scene29_init();
	void scene29_updateHotspots();
	void scene29_run();
	void scene29_updateAnimations();

	// Scene 30
	int _s30_kidSequenceId;
	int scene30_init();
	void scene30_updateHotspots();
	void scene30_run();
	void scene30_updateAnimations();
	
	// Scene 31
	int _s28_currClerkSequenceId, _s28_nextClerkSequenceId;
	bool _s31_beerGuyDistracted;
	int _s31_clerkMeasureCtr, _s31_clerkMeasureMaxCtr;
	int scene31_init();
	void scene31_updateHotspots();
	void scene31_run();
	void scene31_updateAnimations();

	// Scene 32
	int scene32_init();
	void scene32_updateHotspots();
	void scene32_run();
	void scene32_updateAnimations();
	
	// Scene 33
	int _s33_currChickenSequenceId, _s33_nextChickenSequenceId;
	int scene33_init();
	void scene33_updateHotspots();
	void scene33_run();
	void scene33_updateAnimations();

	// Scene 38
	int scene38_init();
	void scene38_updateHotspots();
	void scene38_run();
	void scene38_updateAnimations();

	// Scene 39
	int _s39_currGuySequenceId, _s39_nextGuySequenceId;
	int scene39_init();
	void scene39_updateHotspots();
	void scene39_run();
	void scene39_updateAnimations();

	// Scene 40
	int scene40_init();
	void scene40_updateHotspots();
	void scene40_run();
	void scene40_updateAnimations();

	// Scene 4x
	void toyUfoSetStatus(int flagNum);
	int toyUfoGetSequenceId();
	bool toyUfoCheckTimer();
	void toyUfoFlyTo(int destX, int destY, int minX, int maxX, int minY, int maxY, int animationIndex);

	// Scene 41
	int _s41_currKidSequenceId, _s41_nextKidSequenceId;
	int _s41_currToyVendorSequenceId, _s41_nextToyVendorSequenceId;
	int scene41_init();
	void scene41_updateHotspots();
	void scene41_run();
	void scene41_updateAnimations();

	// Scene 42
	int _s42_currBBQVendorSequenceId;
	int _s42_nextBBQVendorSequenceId;

	int scene42_init();
	void scene42_updateHotspots();
	void scene42_run();
	void scene42_updateAnimations();

	// Scene 43
	int _s43_currTwoHeadedGuySequenceId;
	int _s43_nextTwoHeadedGuySequenceId;

	int scene43_init();
	void scene43_updateHotspots();
	void scene43_run();
	void scene43_updateAnimations();

	// Scene 44
	int _s44_nextSpringGuySequenceId;
	int _s44_nextKissingLadySequenceId;
	int _s44_currSpringGuySequenceId;
	int _s44_currKissingLadySequenceId;
	int scene44_init();
	void scene44_updateHotspots();
	void scene44_run();
	void scene44_updateAnimations();
	
	// Scene 45
	int _s45_currDancerSequenceId;
	int scene45_init();
	void scene45_updateHotspots();
	void scene45_run();
	void scene45_updateAnimations();

	// Scene 46
	int _s46_currSackGuySequenceId;
	int _s46_nextItchyGuySequenceId;
	int _s46_nextSackGuySequenceId;
	int _s46_currItchyGuySequenceId;
	int scene46_init();
	void scene46_updateHotspots();
	void scene46_run();
	void scene46_updateAnimations();
	
	// Scene 47
	void scene47_initCutscene1();
	void scene47_initCutscene2();
	void scene47_initCutscene3();
	void scene47_initCutscene4();
	void scene47_initCutscene5();
	void scene47_initCutscene6();
	void scene47_initCutscene7();

	// Scene 48
	void scene48_initCutscene();

	// Scene 49
	int _s49_scoreBarPos, _s49_scoreLevel, _s49_scoreBarFlash;
	int _s49_obstacleIndex;
	Scene49Obstacle _s49_obstacles[5];
	int _s49_truckSequenceId, _s49_truckId, _s49_truckLaneNum;
	int scene49_init();
	void scene49_updateHotspots();
	void scene49_checkObstacles();
	void scene49_updateObstacle(int id);
	void scene49_increaseScore(int amount);
	void scene49_decreaseScore(int amount);
	void scene49_refreshScoreBar();
	void scene49_clearObstacle(int index);
	void scene49_run();
	void scene49_updateAnimations();

	// Scene 50
	bool _s50_fightDone;
	int _s50_timesPlayed, _s50_timesPlayedModifier;
	int _s50_attackCounter;
	int _s50_roundNum, _s50_timeRemaining;
	int _s50_leftTongueRoundsWon, _s50_rightTongueRoundsWon;
	int _s50_leftTongueEnergyBarPos, _s50_rightTongueEnergyBarPos;
	int _s50_leftTongueSequenceId, _s50_leftTongueId;
	int _s50_leftTongueNextSequenceId, _s50_leftTongueNextId, _s50_leftTongueNextIdCtr;
	int _s50_rightTongueSequenceId, _s50_rightTongueId;
	int _s50_rightTongueNextSequenceId, _s50_rightTongueNextId, _s50_rightTongueNextIdCtr;
	int _s50_leftTongueEnergy, _s50_rightTongueEnergy;
	int scene50_init();
	void scene50_updateHotspots();
	bool scene50_tongueWinsRound(int tongueNum);
	void scene50_playWinAnim(int tongueNum, bool fightOver);
	void scene50_delayTicks();
	void scene50_initRound();
	bool scene50_updateCountdown();
	void scene50_drawCountdown(int value);
	void scene50_playTonguesIdle();
	void scene50_playRoundAnim(int roundNum);
	bool scene50_updateEnergyBars(int newLeftBarPos, int newRightBarPos);
	void scene50_waitForAnim(int animationIndex);
	int scene50_checkInput();
	int scene50_getRightTongueAction();
	void scene50_updateAnimations();
	int scene50_getRightTongueActionTicks();
	int scene50_getLeftTongueNextId();
	int scene50_getRightTongueNextId();
	void scene50_playWinBadgeAnim(int tongueNum);
	void scene50_run();
	
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
	int _s52_shipCannonFired, _s52_shipCannonPosX, _s52_shipCannonPosY;
	int _s52_shipCannonFiring;
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
	int scene52_shipCannonHitShield(int cannonNum);
	int scene52_shipCannonHitAlien();
	void scene52_shipExplode();
	int scene52_checkAlienRow(int rowNum);
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

	// Scene 99
	int _s99_itemsCount;
	int _s99_resourceIdArr[16];
	int _s99_sequenceCountArr[16];
	int _s99_sequenceIdArr[50];
	byte _s99_canSkip[16];
	int cutscene_init();
	void cutscene_run();
};

} // End of namespace Gnap

#endif // GNAP_H
