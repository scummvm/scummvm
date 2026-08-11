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

#ifndef CARNIVAL_CARNIVAL_H
#define CARNIVAL_CARNIVAL_H

#include "bolt/bolt.h"

namespace Bolt {

namespace Carnival {

class CarnivalEngine;
typedef int16 (CarnivalEngine::*SideShowHandler)(int16 prevBooth);

struct BarkerTable {
	SideShowHandler *handlers;
	int16 count;
	int16 minIndex;
	int16 maxIndex;
};

typedef struct FredEntityState {
	uint16 flags;
	int16 frameCountdown;
	int16 animMode;
	int16 frameIndex;
	byte *animTable;
	int16 direction;
	int32 xPos;
	int32 yPos;
	int32 prevXPos;
	int32 prevYPos;
	int16 speed;
	byte *pathTable;
	int16 pathIndex;

	FredEntityState() {
		flags = 0;
		frameCountdown = 0;
		animMode = 0;
		frameIndex = 0;
		animTable = nullptr;
		direction = 0;
		xPos = 0;
		yPos = 0;
		prevXPos = 0;
		prevYPos = 0;
		speed = 0;
		pathTable = nullptr;
		pathIndex = 0;
	}
} FredEntityState;

// HUCK GAME

typedef struct HuckState {
	int16 levelNumber;
	int16 slotIndex[10];
	int16 levelComplete;
	int16 drawTable1[24];
	int16 drawTable2[24];
	int16 giftCount;
	int16 hasCycle;
	int16 selectionPending;
	int16 selected1Slot;
	int16 selected2Slot;
	int16 selected1SpriteId;
	int16 selected2SpriteId;

	HuckState() {
		levelNumber = 0;
		memset(slotIndex, 0, sizeof(slotIndex));
		levelComplete = 0;
		memset(drawTable1, 0, sizeof(drawTable1));
		memset(drawTable2, 0, sizeof(drawTable2));
		giftCount = 0;
		hasCycle = 0;
		selectionPending = 0;
		selected1Slot = 0;
		selected2Slot = 0;
		selected1SpriteId = 0;
		selected2SpriteId = 0;
	}
} HuckState;

// GEORGE GAME

typedef struct GeorgeEntityState {
	uint16 flags;
	int16 frameCountdown;
	int16 animMode;
	int16 variant;
	int16 frameIndex;
	byte *animTable;
	int32 joyInput;
	int32 x;
	int32 y;
	int32 prevX;
	int32 prevY;
	int32 velX;
	int32 velY;
	int32 accelX;
	int32 accelY;
	byte *pathTable;
	int16 pathIndex;

	GeorgeEntityState() {
		flags = 0;
		frameCountdown = 0;
		animMode = 0;
		variant = 0;
		frameIndex = 0;
		animTable = nullptr;
		joyInput = 0;
		x = 0;
		y = 0;
		prevX = 0;
		prevY = 0;
		velX = 0;
		velY = 0;
		accelX = 0;
		accelY = 0;
		pathTable = nullptr;
		pathIndex = 0;
	}
} GeorgeEntityState;

// SCOOBY GAME

typedef struct ScoobyState {
	int16 levelNumber;
	int16 slotIndex[10];
	int16 levelComplete;
	int16 wallStates[25][4];
	int16 scoobyCell;
	int16 scoobySavedCell;
	int16 leftNeighbor;
	int16 rightNeighbor;
	int16 upNeighbor;
	int16 downNeighbor;
	int16 activeLevel;
	int16 scoobyX;
	int16 scoobyY;
	int16 velocityX;
	int16 velocityY;
	int16 targetVelocityX;
	int16 targetVelocityY;
	int16 transitionTarget;
	int16 currentAnim;
	int16 direction;
	int16 spriteFrameCount;
	byte *frameData[6];

	ScoobyState() {
		levelNumber = 0;

		for (int i = 0; i < 10; i++)
			slotIndex[i] = 0;

		levelComplete = 0;

		for (int i = 0; i < 25; i++)
			for (int j = 0; j < 4; j++)
				wallStates[i][j] = 0;

		scoobyCell = 0;
		scoobySavedCell = 0;
		leftNeighbor = 0;
		rightNeighbor = 0;
		upNeighbor = 0;
		downNeighbor = 0;
		activeLevel = 0;
		scoobyX = 0;
		scoobyY = 0;
		velocityX = 0;
		velocityY = 0;
		targetVelocityX = 0;
		targetVelocityY = 0;
		transitionTarget = 0;
		currentAnim = 0;
		direction = 0;
		spriteFrameCount = 0;

		for (int i = 0; i < 6; i++)
			frameData[i] = nullptr;
	}
} ScoobyState;

typedef struct ScoobyRect {
	int16 left;
	int16 right;
	int16 top;
	int16 bottom;

	ScoobyRect() {
		left = 0;
		right = 0;
		top = 0;
		bottom = 0;
	}
} ScoobyRect;

// TOPCAT GAME

typedef struct TopCatAnim {
	int16 animType;
	int16 animIndex;
	int16 transitionToNextQuestionFlag;
	int16 *seqPtr;

	TopCatAnim() {
		animType = 0;
		animIndex = 0;
		transitionToNextQuestionFlag = 0;
		seqPtr = nullptr;
	}
} TopCatAnim;

// YOGI GAME

typedef struct YogiState {
	int16 levelNumber;
	int16 currentSlot;
	int16 levelIndex[10];
	int16 slotIndex[10];
	int16 levelComplete;
	int16 basketSound[24];
	int16 basketState[24];
	int16 basketCount;
	int16 matchCount;
	int16 selectionPending;
	int16 selected1Slot;
	int16 selected2Slot;
	int16 sound1;
	int16 sound2;

	YogiState() {
		levelNumber = 0;
		currentSlot = 0;
		memset(levelIndex, 0, sizeof(levelIndex));
		memset(slotIndex, 0, sizeof(slotIndex));
		levelComplete = 0;
		memset(basketSound, 0, sizeof(basketSound));
		memset(basketState, 0, sizeof(basketState));
		basketCount = 0;
		matchCount = 0;
		selectionPending = 0;
		selected1Slot = 0;
		selected2Slot = 0;
		sound1 = 0;
		sound2 = 0;
	}
} YogiState;

class CarnivalEngine : public BoltEngine {
public:
	CarnivalEngine(OSystem *syst, const ADGameDescription *gameDesc);

protected:
	// xpMain
	void boltMain() override;
	int16 displayDemoPict(int16 prevBooth);

	void initCallbacks() override;
	
	// Booth
	void startCycle(byte *cycleResource);
	void displayBooth(int16 page);
	void playAVOverBooth(int16 animIndex);
	int16 hucksBooth(int16 prevBooth);
	int16 fredsBooth(int16 prevBooth);
	int16 scoobysBooth(int16 prevBooth);
	int16 yogisBooth(int16 prevBooth);
	int16 georgesBooth(int16 prevBooth);
	int16 topCatsBooth(int16 prevBooth);
	int16 mainEntrance(int16 prevBooth);
	bool loadBooth(int16 boothId);
	void unloadBooth();
	int16 openBooth(int16 boothId);
	void closeBooth();
	void playTour();
	void finishPlayingHelp(int16 activeHotspot);
	int16 hotSpotActive(int16 hotspot);
	void hoverHotSpot();
	int16 boothEventLoop();
	void resetInactivityState();
	bool handleButtonPress(int16 hotspot);
	void blastColors(byte **paletteTable, int16 index, int16 mode);
	void setColors(int16 index);
	void restoreColors(int16 index);
	void loadColors();
	void shiftColorMap(byte *colorMap, int16 delta);
	void playBoothAV();
	void mainEntranceHelpBlink();
	void boothHelpBlink();
	void tourPaletteCycleStep();
	void fadeToBlack(int16 steps);
	void flushInput();
	int16 winALetter(int16 prevBooth);
	int16 endDemo(int16 prevBooth);

	int16 _lettersWon = 0;
	bool _allLettersWonFlag = false;
	int _displayMode = 0;
	int32 _displayX = 0;
	int32 _displayY = 0;
	int32 _displayWidth = 0;
	int32 _displayHeight = 0;

	int16 _boothLoadedMask = 0;
	int16 _currentBoothScene = 0;
	int16 _boothNumHotspots = 0;
	int16 _boothNumAnimations = 0;
	byte *_boothSceneDesc = nullptr;
	byte *_boothHotPalDescs[4];
	Common::Rect _boothHotspotDescs[8]; 
	byte *_boothAnimDescs[7];
	byte *_boothAnimPalDescs[4];
	byte _savedPalettes[7][30];
	byte _savedHotPalettes[4][9];
	byte *_boothPalCycleData = nullptr;
	XPPicDesc _boothLetterSprite;
	bool _needInitCursorPos = true;
	byte *_boothVisitSignOn = nullptr;
	byte *_boothVisitSignOff = nullptr;
	byte _leftDoorNavTable[3]  = { 3, 2, 4 };
	byte _rightDoorNavTable[3] = { 1, 0, 5 };

	int16 _cursorX = 0;
	int16 _cursorY = 0;
	int16 _hoveredHotspot = 0;
	uint32 _helpTimer = 0;
	int16 _keyReleased = 0;
	int16 _keyLeft = 0;
	int16 _keyRight = 0;
	int16 _keyUp = 0;
	int16 _keyDown = 0;
	int16 _helpFlag = 0;
	int16 _keyEnter = 0;

	int16 _tourStep = 0;
	int16 _helpPlaying = 0;
	int16 _helpIsIdle = 0;
	int16 _idleHelpAudioAvailable = 1;

	int16 _huckWins = 0;
	int16 _fredWins = 0;
	int16 _scoobyWins = 0;
	int16 _yogiWins = 0;
	int16 _georgeWins = 0;
	int16 _topCatWins = 0;

	// Barker
	BarkerTable *createBarker(int16 minIndex, int16 maxIndex);
	void freeBarker(BarkerTable *table);
	bool registerSideShow(BarkerTable *table, SideShowHandler handler, int16 boothId);
	int16 barker(BarkerTable *table, int16 startBooth);
	bool checkError();

	int16 _currentBoothId = 0;

	// --- MINIGAMES ---

	// --- FRED ---
	int16 fredGame(int16 prevBooth);
	bool initFred();
	void cleanUpFred();
	bool initFredLevel(int16 levelGroup, int16 palGroup);
	void termFredLevel(int16 levelGroup, int16 palGroup);
	void swapFredAnimEntry();
	void swapFredAnimDesc();
	void swapFredLevelDesc();
	int16 playFred();
	int16 helpFred();
	void hiliteFredHelpObject(byte *entry, int16 highlight);
	void helpAnimStep();
	bool spawnBalloon();
	int16 calcBalloonSpawnDelay();
	int16 selectBalloonRow();
	void setFredAnimMode(FredEntityState *state, int16 mode);
	void renderFredScene();
	void getFredSoundInfo(BOLTLib *lib, int16 memberId, SoundInfo *soundInfo);
	void playFredSound(SoundInfo *oneShot, SoundInfo *loop);
	void updateFredSound();

	static void resolveAllRefsCb();
	static void swapFredAnimEntryCb();
	static void swapFredAnimDescCb();
	static void swapFredLevelDescCb();

	BOLTLib *_fredBoltLib = nullptr;
	BOLTCallbacks _fredBoltCallbacks;

	static BOLTCallback _fredTypeLoadCallbacks[28];
	static BOLTCallback _fredTypeFreeCallbacks[28];

	byte *_fredLevelPtr = nullptr;
	byte *_fredBackground = nullptr;
	byte *_fredBalloonString = nullptr;
	byte *_fredPalette = nullptr;
	byte *_fredFacingLeftRect = nullptr;
	byte *_fredFacingRightRect = nullptr;
	byte *_fredTurningRect = nullptr;
	byte *_fredBalloonRect = nullptr;
	byte *_fredHelpEntries = nullptr;
	byte *_fredPlayButton = nullptr;
	byte *_fredSprites[10] = { nullptr };
	FredEntityState **_fredEntitiesTable = nullptr;

	byte *_fredRowBounds = nullptr;
	byte *_fredShuffleTable = nullptr;
	byte *_fredCycleRaw = nullptr;
	byte *_fredBalloonSprite = nullptr;
	byte *_fredPathMatrix = nullptr;
	byte *_fredCurrentHelpObject = nullptr;
	byte *_fredHoveredEntry = nullptr;

	SoundInfo _fredSounds[4];

	SoundInfo *_fredCurrentSound = nullptr;
	SoundInfo *_fredLoopSound = nullptr;
	SoundInfo *_fredPendingOneShot = nullptr;
	SoundInfo *_fredPendingLoop = nullptr;

	FredEntityState _fredSprite;

	int16 _fredSaveData[3] = { 0, 0, 0 };
	int16 _fredLevelIndex = 0;

	int16 _fredBalloonSpawnDelay = 0;
	int16 _fredBalloonSearchIdx = 0;
	uint32 _fredTimer = 0;
	int16 _fredHelpStep = -1;
	int16 _fredShowHelp = 1;

	XPCycleState _fredCycleSpecs[4];

	// --- GEORGE ---
	int16 georgeGame(int16 prevBooth);
	bool initGeorge();
	void cleanUpGeorge();
	bool initGeorgeLevel(int16 level, int16 variant);
	void termGeorgeLevel(int16 level, int16 variant);
	void swapGeorgeFrameArray();
	void swapGeorgeHelpEntry();
	void swapGeorgeThresholds();
	int16 playGeorge();
	int16 helpGeorge();
	void hiliteGeorgeHelpObject(byte *entry, int16 highlight);
	void advanceHelpAnimation();
	bool spawnSatellite();
	int16 getRandomSatelliteWait();
	int16 getRandomAsteroidRow();
	bool confirmAsteroidHitTest();
	bool spawnAsteroid();
	int16 getRandomAsteroidWait();
	int16 getAsteroidRow();
	void setGeorgeAnimMode(GeorgeEntityState *sat, int16 mode);
	void setSatelliteAnimMode(GeorgeEntityState *sat, int16 mode, int16 variant);
	void setAsteroidAnimMode(GeorgeEntityState *sat, int16 mode, int16 variant);
	void drawFlyingObjects();
	void getGeorgeSoundInfo(BOLTLib *boltLib, int16 member, SoundInfo *outInfo, byte priority);
	void playGeorgeSound(SoundInfo *newSound, SoundInfo *nextSound);
	void updateGeorgeSound();

	static void swapGeorgeFrameArrayCb();
	static void swapGeorgeHelpEntryCb();
	static void swapGeorgeThresholdsCb();

	BOLTLib *_georgeBoltLib = nullptr;
	BOLTCallbacks _georgeBoltCallbacks;

	static BOLTCallback _georgeTypeLoadCallbacks[28];
	static BOLTCallback _georgeTypeFreeCallbacks[28];

	byte *_georgePrevActiveHelpObject = nullptr;
	byte *_georgeActiveHelpObject = nullptr;
	int16 _georgeHelpStep = -1;
	int16 _georgeHelpActive = 1;

	GeorgeEntityState **_georgeEntityList = nullptr;
	int16 _georgeCarIdx = 0;
	int16 _georgeNumSatellites = 0;
	int16 _georgeFirstAsteroidIdx = 0;
	int16 _georgeTotalSatellites = 0;
	byte *_georgeCarPics[3] = { nullptr };
	byte *_georgeHelpObjects = nullptr;
	byte *_georgeHelpSequence = nullptr;
	uint32 _georgeHelpTimer = 0;
	byte *_georgeSatelliteShuffleTable = nullptr;
	byte *_georgeAsteroidShuffleTable = nullptr;
	int16 *_georgeSatelliteThresholds = nullptr;
	int16 *_georgeAsteroidThresholds = nullptr;
	byte *_georgeSatellitePaths = nullptr;
	byte *_georgeAsteroidPaths = nullptr;
	byte *_georgeCollisionRect = nullptr;
	byte *_georgeSatelliteCollisionRects = nullptr;
	byte *_georgeAsteroidCollisionRects = nullptr;
	byte *_georgeSatelliteGfx = nullptr;
	byte *_georgeAsteroidGfx = nullptr;
	int16 _georgeSatelliteWait = 0;
	int16 _georgeAsteroidWait = 0;
	int16 _georgeSatelliteSearchIdx = 0;
	int16 _georgeAsteroidSearchIdx = 0;
	int16 _georgeHitSearchIdx = 0;
	byte *_georgeBgPic = nullptr;
	byte *_georgePalette = nullptr;
	byte *_georgePalCycleRawData = nullptr;
	XPCycleState _georgePalCycleSpecs[4];
	int16 _georgeCollectedSatellitesNum = 0;
	SoundInfo _georgeSoundCarStartUp;
	SoundInfo _georgeSoundCarGoesAway;
	SoundInfo _georgeSoundCarLoopHi;
	SoundInfo _georgeSoundCarLoopLo;
	SoundInfo _georgeSoundCarTumble;
	SoundInfo **_georgeSatelliteSoundList = nullptr;
	byte _georgeSoundChannelCounter = 0;
	SoundInfo *_georgeSoundToPlay = nullptr;
	SoundInfo *_georgeSoundCurrent = nullptr;
	SoundInfo *_georgeSoundQueued = nullptr;
	SoundInfo *_georgeSoundNext = nullptr;
	int16 _georgeSaveData[3] = { 0 };
	int16 *_georgeThresholds = nullptr;

	// --- HUCK ---
	void playSoundMapHuck(int16 memberId);
	void waitSoundMapHuck();
	void setHuckColors(int16 which);
	void restoreHuckColors(int16 which);
	void startHuckShuffleTimer();
	void drawGift(int16 slot);
	void drawHuckGifts();
	void checkHuckLevelComplete();
	bool initHuckDisplay();
	bool loadHuckResources();
	void unloadHuckResources();
	bool initHuckLevel();
	bool resumeHuckLevel();
	bool initHuck();
	void huckToggleBlinking(int16 *state, int16 which);
	void huckUpdateHotSpots(int16 x, int16 y);
	int16 findGift(int16 x, int16 y);
	bool handleGiftSelect(int16 x, int16 y);
	void huckHandleActionButton(int16 x, int16 y);
	void giftSwap();
	void resolveHuckSelection();
	void handleEvent(int16 eventType, uint32 eventData);
	void playHuck();
	void cleanUpHuck();
	int16 huckGame(int16 prevBooth);
	void swapHuckWordArray();
	void swapHuckWords();

	static void swapHuckWordArrayCb();
	static void swapHuckWordsCb();

	BOLTLib *_huckBoltLib = nullptr;
	BOLTCallbacks _huckBoltCallbacks;

	static BOLTCallback _huckTypeLoadCallbacks[27];
	static BOLTCallback _huckTypeFreeCallbacks[27];

	int16 _huckSoundPlaying = 0;
	int16 _huckHotSpotCount = 0;
	int16 _huckActionState = 0;
	uint32 _huckScreensaverTimer = 0;
	uint32 _huckBlinkTimer = 0;
	int16 _huckScreensaverFlag = 0;
	int16 _huckBlinkFlag = 0;
	uint32 _huckShuffleTimer = 0;
	int16 _huckGlobal[30] = { 0 };
	HuckState _huckState;
	byte *_huckGiftPic = nullptr;
	byte *_huckBgPic = nullptr;
	int16 _huckGiftGroupId = 0;
	int16 _huckVariantGroupId = 0;
	byte *_huckBgDisplayPic = nullptr;
	int16 _huckScrollOffset = 0;
	int16 _huckPalRange[8] = { 0 };
	int16 _huckReturnBooth = 0;
	int16 _huckExitFlag = 0;
	int16 _huckCursorX = 0;
	int16 _huckCursorY = 0;
	byte _huckPalSave0[15] = { 0 };
	byte _huckPalHighlight0[15] = { 0 };
	byte _huckPalSave1[15] = { 0 };
	byte _huckPalHighlight1[15] = { 0 };
	XPPicDesc _huckScratchPic;

	// --- SCOOBY ---
	bool loadScoobyBaseAssets();
	void cleanUpScoobyBaseAssets();
	void displayPicClipHack(byte *pic, int16 offsetX, int16 offsetY, int16 *clipRect, int16 displayMode);
	void drawMovingWalls(int16 cellIdx, int16 direction, int16 picFrame, int16 bgFrame);
	void drawAllMovingWalls();
	void animateTransition(int16 memberIdx);
	void clearPictMSb(byte *pic);
	void initScoobyLevelGraphics();
	bool initScoobyLevelAssets();
	void cleanUpScoobyLevelGraphics();
	void setScoobySpriteDirection(int16 startMember);
	void playSoundMapScooby(int16 memberIdx);
	void playWallSound();
	void animateWalls();
	void decideDirection();
	void updateScoobySound();
	void setScoobySound(int16 mode);
	void updateScoobyLocation();
	void updateScoobyWalls();
	void updateScoobyDirection(int16 inputDir);
	void updateScoobyTransition();
	bool initScoobyLevel();
	bool resumeScoobyLevel();
	bool initScooby();
	void cleanUpScooby();
	int16 helpScooby();
	void hiliteScoobyHelpObject(byte *entry, int16 highlight);
	int16 xpDirToBOLTDir(uint32 xpDir);
	int16 playScooby();
	int16 scoobyGame(int16 prevBooth);
	void swapScoobyHelpEntry();
	void swapScoobyWordArray();

	static void swapScoobyHelpEntryCb();
	static void swapScoobyWordArrayCb();

	BOLTLib *_scoobyBoltLib = nullptr;
	BOLTCallbacks _scoobyBoltCallbacks;

	static BOLTCallback _scoobyTypeLoadCallbacks[28];
	static BOLTCallback _scoobyTypeFreeCallbacks[28];

	int16 _scoobySoundMode = 0;
	int16 _scoobySoundPlaying = 0;
	int16 _scoobyShowHelp = 1;

	int16 _scoobyLastInputDir = 0;
	int16 _scoobyTransitionFrom = 0;
	int16 _scoobyTransitionTarget = 0;
	int16 _scoobyTransitionVelX = 0;
	int16 _scoobyTransitionVelY = 0;
	XPPicDesc _scoobyTempPic;
	byte _scoobyGlobalSaveData[0x3C] = { 0 };
	ScoobyState _scoobyGameState;
	SSprite _scoobySprite;
	byte *_scoobyBaseData = nullptr;
	byte *_scoobyBgPic = nullptr;
	byte *_scoobyWallPicsA[4] = { nullptr };
	byte *_scoobyWallPicsB[5] = { nullptr };
	ScoobyRect _scoobyCellBounds[25];
	Common::Point _scoobyLevelStartXY[25];
	byte *_scoobyLevelData = nullptr;
	int16 _scoobySelectedGraphicsGroup = 0;
	int16 _scoobyDifficulty = 0;
	int16 _scoobyLevelCount = 0;
	int16 _scoobyMoveRequested = 0;
	int16 _scoobyTransitioning = 0;
	int16 _scoobyDesiredDir = 0;
	int16 _scoobyInputHoldCount = 0;
	int16 _scoobyWallAnimating = 0;
	int16 _scoobyWallAnimStep = 0;
	int16 _scoobyWallsToOpen = 0;
	int16 _scoobyWallsToClose = 0;

	// --- TOPCAT ---
	int16 topCatGame(int16 prevBooth);
	bool initTopCat();
	void cleanUpTopCat();
	int16 playTopCat();
	int16 handleActionButton(int16 *result);
	void queueAnim(int16 animType, int16 param);
	bool maintainAnim(int16 soundEvent);
	void maintainIdleSound(int16 decrement);
	bool startNextAnim(int16 *playAnswerAnim);
	void setAnimType(int16 newType);
	void highlightObject(byte *entry, int16 highlight);
	bool setupNextQuestion();
	void adjustColors(byte *pic, int8 shift);
	void shuffleTopCatQuestions();
	void shuffleTopCatPermutations();
	void getTopCatSoundInfo(BOLTLib *lib, int16 memberId, SoundInfo *soundInfo);
	void setScoreLights(int16 litMask);
	void swapTopCatHelpEntry();

	static void swapTopCatHelpEntryCb();

	RTFResource *_topCatRtfHandle = nullptr;
	RTFResource *_topCatAvHandle = nullptr;
	BOLTLib *_topCatBoltLib = nullptr;
	BOLTCallbacks _topCatBoltCallbacks;

	static BOLTCallback _topCatTypeLoadCallbacks[26];
	static BOLTCallback _topCatTypeFreeCallbacks[26];

	byte *_topCatBackgroundPalette = nullptr;
	byte *_topCatBackground = nullptr;
	byte *_topCatBackgroundAnimationPalette = nullptr;
	byte *_topCatGraphicsAssets = nullptr;

	int16 _topCatBackgroundAnimFrame = 0;
	int16 _topCatMaxBackgroundAnimFrames = 0;
	int16 _topCatCurrentAnimType = 0;
	int16 _topCatAnimStateMachineStep = 0;
	int16 _topCatAnimQueueSize = 0;
	int16 _topCatQueuedSoundFrames = 0;
	byte *_topCatButtonsPalette = nullptr;
	byte *_topCatBlinkEntries = nullptr;
	byte *_topCatLightsPalette = nullptr;
	int16 *_topCatBlinkSeqPtr = nullptr;
	byte *_topCatSelectedChoiceOverlayGfx = nullptr;
	byte *_topCatCycleData = nullptr;
	XPCycleState _topCatCycleSpecs[4];
	byte *_topCatShuffledQuestions = nullptr;
	byte *_topCatShuffledAnswers = nullptr;
	byte *_topCatAnswersPermutations = nullptr;
	byte *_topCatAnswers = nullptr;
	byte *_topCatAnswersScreenPositions = nullptr;
	int16 _topCatSavedScore = 0;
	int16 _topCatSaveHistory = 0;
	int16 _topCatScore = 0;
	int16 _topCatShuffledQuestionsArrayIdx = 0;
	uint32 _topCatBlinkTimer = 0;
	SoundInfo _topCatSoundInfo;
	byte _topCatSavedShuffledQuestions[60] = { 0 };
	byte _topCatSavedShuffledAnswers[60] = { 0 };
	byte _topCatSavedAnswersPermutations[60 * 3] = { 0 };
	byte _topCatSaveBuffer[302] = { 0 };
	byte *_topCatHoveredEntry = nullptr;
	byte *_topCatHelpButton = nullptr;
	byte *_topCatBackButton = nullptr;
	byte *_topCatBlinkEntry = nullptr;
	int16 _topCatCycleStep = 0;
	int16 _topCatDrawnQuestionId = 0;
	int16 _topCatCurrentAnswerIdx = 0;
	XPCycleState _topCatChoiceCycleState[4];
	TopCatAnim _topCatAnimQueue[3];
	int16 _topCatCorrectAnimIdx = 0;
	int16 _topCatWrongAnimIdx = 0;
	int16 _topCatShouldPlayAnswerAnim = 0;
	int16 _topCatMaintainSoundFlag = 0;
	int16 _topCatPermTableA[3] = { 0, 1, 2 };
	int16 _topCatPermTableB[3] = { 0, 1, 2 };
	int16 _topCatBlinkSeq0[5] = { 0x01, 0x00, 0x01, 0x00, 0x01 };
	int16 _topCatBlinkSeq1[5] = { 0x03, 0x01, 0x03, 0x01, 0x03 };
	int16 _topCatBlinkSeq2[5] = { 0x07, 0x03, 0x07, 0x03, 0x07 };
	int16 _topCatBlinkSeq3[5] = { 0x0F, 0x07, 0x0F, 0x07, 0x0F };
	int16 _topCatBlinkSeq4[5] = { 0x1F, 0x0F, 0x1F, 0x0F, 0x1F };
	int16 _topCatBlinkSeq5[5] = { 0x3F, 0x1F, 0x3F, 0x1F, 0x3F };
	int16 _topCatBlinkSeq6[25] = {
		0x09, 0x12, 0x24, 0x09, 0x12, 0x24,
		0x09, 0x12, 0x24, 0x09, 0x12, 0x24,
		0x09, 0x12, 0x24, 0x09, 0x12, 0x24,
		0x09, 0x12, 0x24, 0x09, 0x12, 0x24,
		0x3F
	};

	int16 *_topCatBlinkSeqTable[7] = {
		_topCatBlinkSeq0, _topCatBlinkSeq1, _topCatBlinkSeq2,
		_topCatBlinkSeq3, _topCatBlinkSeq4, _topCatBlinkSeq5,
		_topCatBlinkSeq6
	};

	// --- YOGI ---
	void playSoundMapYogi(int16 memberId);
	void waitSoundMapYogi();
	void stopSoundYogi();
	void setYogiColors(int16 which);
	void restoreYogiColors(int16 which);
	void drawBasket(int16 slot, byte *basketSprite);
	void drawAllBaskets();
	void handleYogiMatch();
	bool loadYogiBgPic();
	void unloadYogiBgPic();
	void drawYogiLevel();
	bool loadYogiLevel();
	void unloadYogiResources();
	bool initYogiLevel();
	bool resumeYogiLevel();
	bool initYogi();
	void yogiToggleBlinking(int16 which, int16 *state);
	void yogiUpdateHotSpots(int16 x, int16 y);
	int16 findBasket(int16 x, int16 y);
	void resolveYogiSelection();
	bool handleBasketSelect(int16 x, int16 y);
	void yogiHandleActionButton(int16 x, int16 y);
	void handleYogiEvent(int16 eventType, uint32 eventData);
	void playYogi();
	void cleanUpYogi();
	int16 yogiGame(int16 prevBooth);

	void swapYogiAllWords();
	void swapYogiFirstWord();

	static void swapYogiAllWordsCb();
	static void swapYogiFirstWordCb();

	BOLTLib *_yogiBoltLib = nullptr;
	BOLTCallbacks _yogiBoltCallbacks;

	static BOLTCallback _yogiTypeLoadCallbacks[27];
	static BOLTCallback _yogiTypeFreeCallbacks[27];

	int16 _yogiSoundPlaying = 0;
	int16 _yogiSoundActive = 0;
	int16 _yogiHotSpotCount = 0;
	int16 _yogiAnimActive = 0;
	uint32 _yogiBlinkTimer1 = 0;
	uint32 _yogiBlinkTimer2 = 0;
	int16 _yogiBlinkState1 = 0;
	int16 _yogiBlinkState2 = 0;
	int16 _yogiGlobal[0xA0] = { 0 };
	YogiState _yogiState;
	byte *_yogiBasketPic = nullptr;
	int16 _yogiLevelGroupId = 0;
	byte *_yogiBgPic = nullptr;
	byte *_yogiNormalSprite = nullptr;
	byte *_yogiHlSprite = nullptr;
	byte *_yogiAnimSprite = nullptr;
	int16 _yogiSpriteStride = 0;
	int16 _yogiSpriteHeight = 0;
	int16 _yogiPalRange[8] = { 0 };
	int16 _yogiReturnBooth = 0;
	int16 _yogiExitFlag = 0;
	int16 _yogiLevelId = 0;
	int16 _yogiCursorX = 0;
	int16 _yogiCursorY = 0;
	byte _yogiPalSave0[15] = { 0 };
	byte _yogiPalHighlight0[15] = { 0 };
	byte _yogiPalSave1[15] = { 0 };
	byte _yogiPalHighlight1[15] = { 0 };
	XPPicDesc _yogiScratchBuf;
};

} // End of namespace Carnival

} // End of namespace Bolt

#endif // CARNIVAL_CARNIVAL_H
