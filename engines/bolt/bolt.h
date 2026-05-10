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

#ifndef BOLT_H
#define BOLT_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "bolt/detection.h"
#include "bolt/xplib/xplib.h"

namespace Bolt {

#define SCREEN_WIDTH           320
#define SCREEN_HEIGHT          200
#define EXTENDED_SCREEN_WIDTH  384
#define EXTENDED_SCREEN_HEIGHT 240

struct BoltGameDescription;
class XpLib;

typedef struct DisplaySpecs {
	int16 id;
	int16 width;
	int16 height;
} DisplaySpecs;

typedef int16 (BoltEngine::*SideShowHandler)(int16 prevBooth);

struct BarkerTable {
	SideShowHandler *handlers;
	int16 count;
	int16 minIndex;
	int16 maxIndex;
};

typedef struct BOLTHeader {
	byte header[11];
	byte groupCount;
	uint32 groupDirOffset;

	BOLTHeader() {
		memset(header, 0, sizeof(header));
		groupCount = 0;
		groupDirOffset = 0;
	}
} BOLTHeader;

typedef void (*BOLTCallback)(void);

typedef struct BOLTCallbacks {
	BOLTCallback *typeLoadCallbacks;
	BOLTCallback *typeFreeCallbacks;
	BOLTCallback *memberLoadCallbacks;
	BOLTCallback *memberFreeCallbacks;
	BOLTCallback *groupLoadCallbacks;
	BOLTCallback *groupFreeCallbacks;

	BOLTCallbacks() {
		typeLoadCallbacks = nullptr;
		typeFreeCallbacks = nullptr;
		memberLoadCallbacks = nullptr;
		memberFreeCallbacks = nullptr;
		groupLoadCallbacks = nullptr;
		groupFreeCallbacks = nullptr;
	}
} BOLTCallbacks;

typedef struct BOLTMemberEntry {
	byte flags;
	byte preLoadCbIndex;
	byte preFreeCbIndex;
	byte typeCbIndex;
	uint32 decompSize;
	uint32 fileOffset;
	uint32 dataPtrPlaceholder;
	byte *dataPtr;

	BOLTMemberEntry() {
		flags = 0;
		preLoadCbIndex = 0;
		preFreeCbIndex = 0;
		typeCbIndex = 0;
		decompSize = 0;
		fileOffset = 0;
		dataPtrPlaceholder = 0;
		dataPtr = nullptr;
	}
} BOLTMemberEntry;

typedef struct BOLTGroupEntry {
	byte flags;
	byte loadCbIndex;
	byte freeCbIndex;
	byte memberCount;
	uint32 memberDirOffset;
	uint32 memberDataOffset;
	uint32 groupDataPtrPlaceholder;
	byte *memberData;
	BOLTMemberEntry *members;

	BOLTGroupEntry() {
		flags = 0;
		loadCbIndex = 0;
		freeCbIndex = 0;
		memberCount = 0;
		memberDirOffset = 0;
		memberDataOffset = 0;
		groupDataPtrPlaceholder = 0;
		memberData = nullptr;
		members = nullptr;
	}

	~BOLTGroupEntry() {
		free(memberData);
		memberData = nullptr;

		delete[] members;
		members = nullptr;
	}

	void initMembers(int numMembers) {
		int actualNumber = numMembers == 0 ? 256 : numMembers;
		members = new BOLTMemberEntry[actualNumber];
	}

} BOLTGroupEntry;

typedef struct BOLTLib {
	int16 refCount;
	int16 groupCount;
	Common::File *fileHandle;
	BOLTCallbacks callbacks;
	BOLTGroupEntry *groups;

	BOLTLib(int inGroupCount) {
		refCount = 0;
		groupCount = 0;
		fileHandle = nullptr;
		groups = new BOLTGroupEntry[inGroupCount];
	}

	~BOLTLib() {
		delete[] groups;
	}
} BOLTLib;

typedef struct RTFResource {
	Common::File *fileHandle;
	uint32 *indexTablePtr;
	int16 entryCount;
	byte *indexTableRawData;
	uint32 *indexTable;

	RTFResource() {
		fileHandle = nullptr;
		indexTablePtr = nullptr;
		entryCount = 0;
		indexTableRawData = nullptr;
		indexTable = nullptr;
	}

	~RTFResource() {
		if (indexTable)
			delete[] indexTable;
	}

	void reserveTableSize(int numElements) {
		indexTable = new uint32[numElements];
	}
} RTFResource;

struct RTFPacket;
typedef struct RTFPacket {
	uint32 tag;
	uint32 allocSize;
	uint32 dataSize;
	byte *dataPtr;
	int16 skipCount;
	int16 frameRate;
	int16 duration;
	int16 timestamp;
	RTFPacket *next;
	byte *ringBufPtr;

	RTFPacket() {
		tag = 0;
		allocSize = 0;
		dataSize = 0;
		dataPtr = nullptr;
		skipCount = 0;
		frameRate = 0;
		duration = 0;
		timestamp = 0;
		next = nullptr;
		ringBufPtr = nullptr;
	}
} RTFPacket;

struct SSprite;

typedef void (*SSpriteUpdateFunc)(void);
typedef void (*SSpritePathCallback)(SSprite *);

typedef struct SSprite {
	int16 flags;
	int16 x;
	int16 y;
	int16 frameCount;
	int16 currentFrame;
	byte **frameData;
	SSpriteUpdateFunc updateFunc;
	SSpritePathCallback pathCallback;
	int16 frameTimer;
	int16 frameRate;
	int16 startFrame;
	int16 stopFrame;
	int16 velocityX;
	int16 velocityY;
	int16 xLimitLow;
	int16 xLimitHigh;
	int16 yLimitLow;
	int16 yLimitHigh;
	int16 accelX;
	int16 accelY;
	int16 dragX;
	int16 dragY;
	byte *pathData;
	int16 pathOffset;
	int16 pathLength;
	int16 collX;
	int16 collY;
	int16 collW;
	int16 collH;
	int16 userInfo;

	SSprite() {
		flags = 0;
		x = 0;
		y = 0;
		frameCount = 0;
		currentFrame = 0;
		frameData = nullptr;
		updateFunc = nullptr;
		pathCallback = nullptr;
		frameTimer = 0;
		frameRate = 0;
		startFrame = 0;
		stopFrame = 0;
		velocityX = 0;
		velocityY = 0;
		xLimitLow = 0;
		xLimitHigh = 0;
		yLimitLow = 0;
		yLimitHigh = 0;
		accelX = 0;
		accelY = 0;
		dragX = 0;
		dragY = 0;
		pathData = nullptr;
		pathOffset = 0;
		pathLength = 0;
		collX = 0;
		collY = 0;
		collW = 0;
		collH = 0;
		userInfo = 0;
	}
} SSprite;

// FRED GAME

typedef struct SoundInfo {
	byte *data;
	uint32 size;
	byte priority;
	byte channel;

	SoundInfo() {
		data = nullptr;
		size = 0;
		priority = 0;
		channel = 0;
	}
} SoundInfo;


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

class BoltEngine : public Engine {
friend class XpLib;

private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
protected:
	// Engine APIs
	Common::Error run() override;

public:
	BoltEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~BoltEngine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsReturnToLauncher);
	};

	// This engine doesn't save anything!
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return false;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return false;
	}

protected:
	DisplaySpecs _displaySpecs[2] = {
		{0, 384, 240},
		{1, 320, 200}
	};

	XpLib *_xp = nullptr;
	bool _extendedViewport = false;
	bool _isDemo = false;

	// xpMain
	void boltMain();
	void setCursorPict(byte *sprite);
	int16 displayDemoPict(int16 prevBooth);

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

	int32 _curErrorCode = 0;
	int16 _currentBoothId = 0;

	// Utils
	void displayColors(byte *palette, int16 page, int16 flags);
	byte getPixel(byte *sprite, int16 localX, int16 localY);
	void boltPict2Pict(XPPicDesc *dest, byte *boltSprite);
	void displayPic(byte *boltSprite, int16 xOff, int16 yOff, int16 page);
	bool pointInRect(Common::Rect *rect, int16 x, int16 y);
	const char *assetPath(const char *fileName);
	void boltCycleToXPCycle(byte *srcData, XPCycleState *cycleDesc);
	void unpackColors(int16 count, byte *packedColors);
	bool intersectRect(const Common::Rect *a, const Common::Rect *b, Common::Rect *out);

	// Swap
	void swapPicHeader();
	void swapAndResolvePicDesc();
	void swapFirstWord();
	void swapFirstTwoWords();
	void swapFirstFourWords();
	void swapSpriteHeader();
	void freeSpriteCleanUp();

	// Resource
	bool libRead(Common::File *fileHandle, uint32 offset, byte *dest, uint32 size);
	void decompress(byte *dest, uint32 decompSize, byte *src);
	void resolveIt(uint32 *ref);
	void resolvePendingFixups();
	void resolveFunction(uint32 *ref);
	void resolveAllRefs();
	byte *getResolvedPtr(byte *data, int offset);
	bool openBOLTLib(BOLTLib **outLib, BOLTCallbacks *outIdx, const char *fileName);
	bool closeBOLTLib(BOLTLib **lib);
	bool attemptFreeIndex(BOLTLib *lib, int16 groupId);
	bool loadGroupDirectory();
	bool getBOLTGroup(BOLTLib *lib, int16 groupId, int16 flags);
	void freeBOLTGroup(BOLTLib *lib, int16 groupId, int16 flags);
	byte *getBOLTMember(BOLTLib *lib, int16 resId);
	bool freeBOLTMember(BOLTLib *lib, int16 resId);
	Common::Rect memberToRect(byte *data);
	byte *memberAddr(BOLTLib *lib, int16 resId);
	byte *memberAddrOffset(BOLTLib *lib, uint32 resIdAndOffset);
	uint32 memberSize(BOLTLib *lib, int16 resId);
	byte *groupAddr(BOLTLib *lib, int16 groupId);
	bool allocResourceIndex();
	void freeResourceIndex();
	void swapAllWords();
	void swapAllLongs();

	BOLTLib *_boothsBoltLib = nullptr;
	BOLTCallbacks _boothsBoltCallbacks;

	static BOLTCallback _defaultTypeLoadCallbacks[25];
	static BOLTCallback _defaultTypeFreeCallbacks[25];
	static BOLTCallback _defaultMemberLoadCallbacks[25];
	static BOLTCallback _defaultMemberFreeCallbacks[25];
	static BOLTCallback _defaultGroupLoadCallbacks[25];
	static BOLTCallback _defaultGroupFreeCallbacks[25];

	static void noOpCb();

	static void swapAllWordsCb();
	static void swapAllLongsCb();
	static void swapPicHeaderCb();
	static void swapAndResolvePicDescCb();
	static void swapFirstWordCb();
	static void swapFirstTwoWordsCb();
	static void swapFirstFourWordsCb();
	static void swapSpriteHeaderCb();
	static void freeSpriteCleanUpCb();

	void initCallbacks();

	int16 _resourceIndexCount = 1000;
	uint32 **_resourceIndex = nullptr;
	Common::Array<byte *> _resolvedPtrs;
	Common::File *_cachedFileHandle = nullptr;
	uint32 _cachedFilePos = 0xFFFFFFFF;
	BOLTLib *_boltCurrentLib = nullptr;
	BOLTHeader _boltFileHeader;
	BOLTGroupEntry *_boltCurrentGroupEntry = nullptr;
	int16 _boltLoadDepth = 0;
	byte *_boltRawMemberData = nullptr;
	BOLTMemberEntry *_boltCurrentMemberEntry = nullptr;
	int16 _pendingFixupCount = 0;

	// Game state
	bool initVRam(int16 poolSize);
	void freeVRam();
	bool vLoad(void *dest, const char *name);
	bool vSave(void *src, uint16 srcSize, const char *name);
	bool vDelete(const char *name);
	byte *dataAddress(int16 recordOffset);
	uint16 dataSize(int16 recordOffset);
	bool findRecord(const char *name, int16 *outOffset);

	int32 _vramRecordCount = 0;
	int32 _vramUsedBytes = 0;
	byte *_allocatedMemPool = nullptr;
	uint32 _allocatedMemPoolSize = 0;

	// RTF
	RTFResource *openRTF(const char *fileName);
	void closeRTF(RTFResource *rtf);
	bool playRTF(RTFResource *rtfFile, int16 animIndex, byte *ringBuffer, int32 bufferSize);
	bool fillRTFBuffer();
	void flushRTFSoundQueue();
	bool maintainRTF(int16 mode, RTFPacket **outFrameData);
	bool isRTFPlaying();
	bool killRTF(uint32 *outFilePos);
	bool readPacket();
	void preProcessPacket(RTFPacket *packet);
	void queuePacket(RTFPacket *packet);
	RTFPacket *deQueuePacket();
	RTFPacket *allocPacket(uint32 dataSize);
	void freePacket(RTFPacket *packet);
	void resetPlaybackState();
	void setAVBufferSize(uint32 bufSize);

	RTFResource *_rtfHandle = nullptr;
	Common::File *_rtfFileHandle = nullptr;
	uint32 _rtfChunkRemaining = 0;
	bool _rtfMidChunk = false;
	RTFPacket *_rtfCurrentPacket = nullptr;
	byte *_ringBufBase = nullptr;
	byte *_ringBufWritePtr = nullptr;
	uint32 _ringBufSize = 0;
	uint32 _ringBufFreeSpace = 0;
	uint32 _ringBufLowWater = 0;
	uint32 _ringBufHighWater = 0;
	uint32 _ringBufUsed = 0;
	bool _rtfSoundActive = false;
	int16 _rtfPlaybackTime = 0;
	int16 _rtfCumulativeTime = 0;
	RTFPacket *_rtfPendingFrame = nullptr;
	RTFPacket *_rtfSoundQueueHead = nullptr;
	RTFPacket *_rtfSoundPlayHead = nullptr;
	RTFPacket *_rtfChunkListTail = nullptr;
	RTFPacket *_rtfChunkListHead = nullptr;
	int16 _rtfChunkCount = 0;
	int16 _rtfQueuedSoundCount = 0;
	int16 _rtfSoundTiming = 0;
	uint32 _rtfAnimStartOffset = 0;
	bool _rtfNeedInitialFill = false;
	uint32 _rtfChunkTag = 0;
	uint32 _rtfChunkSize = 0;

	// AV
	bool prepareAV(RTFResource *rtfHandle, int16 animIndex, int16 width, int16 height, int16 xOff, int16 yOff);
	bool maintainAV(int16 mode);
	void stopAV();
	bool playAV(RTFResource *rtfHandle, int16 animIndex, int16 width, int16 height, int16 xOff, int16 yOff);
	void processPacket(RTFPacket *packet);
	void processRL7(RTFPacket *packet);
	void processPLTE(RTFPacket *packet);
	bool initAV(RTFResource *rtfHandle, int16 animIndex, int16 width, int16 height, int16 xOff, int16 yOff);
	void cleanUpAV();

	byte *_avRingBuffer = nullptr;
	uint32 _avTargetBufSize = 0x0FA000;
	int16 _avSkipLevel = 0;
	byte *_avFrontPalette = nullptr;
	byte *_avBackPalette = nullptr;
	uint32 _avSavedInactivityTimer = 0;
	uint32 _avSavedScreenSaverTimer = 0;
	int16 _avFrameAccum = 0;
	int16 _avDisplayX = 0;
	int16 _avDisplayY = 0;

	XPPicDesc _avFrontBufDesc;
	XPPicDesc _avBackBufDesc;

	// Anim
	bool startAnimation(RTFResource *rtf, int16 animIndex);
	void stopAnimation();
	bool maintainAudioPlay(int16 mode);
	bool initAnim(RTFResource *rtf, int16 animIndex);
	void cleanUpAnim();

	int16 _animPrevInactivityTimer = 0;
	byte *_animRingBuffer = nullptr;
	Common::File *_animFileHandle = nullptr;

	// SSprite
	void setUpSSprite(SSprite *sprite, int16 frameCount, byte **frameData, int16 frameRate, int16 velocityX, int16 velocityY);
	void animateSSprite(SSprite *sprite, int16 page);
	void displaySSprite(SSprite *sprite, int16 x, int16 y);
	void eraseSSprite(SSprite *sprite);
	void setSSpriteFrames(SSprite *sprite, int16 frameCount, byte **frameData, int16 frameRate);
	void setSSpriteDrag(SSprite *sprite, int16 dragX, int16 dragY);
	void setSSpriteAccel(SSprite *sprite, int16 accelX, int16 accelY);
	void reverseSSpriteAccel(SSprite *sprite);
	void addSSpriteAccel(SSprite *sprite, int16 dx, int16 dy);
	void setSSpriteVelocity(SSprite *sprite, int16 vx, int16 vy);
	void reverseSSpriteVelocity(SSprite *sprite);
	void setSSpriteStart(SSprite *sprite, int16 startFrame, int16 x, int16 y);
	void setSSpriteStop(SSprite *sprite, int16 stopFrame);
	void setSSpritePath(SSprite *sprite, byte *pathData, int16 pathCount, SSpritePathCallback callback);
	bool inSSprite(SSprite *sprite, int16 x, int16 y);
	bool sSpriteCollide(SSprite *spriteA, SSprite *spriteB);
	void setSSpriteCollision(SSprite *sprite, int16 *bounds);
	bool sSpriteAlive(SSprite *sprite);
	void getSSpriteLoc(SSprite *sprite, Common::Point *out);
	void getSSpriteAccel(SSprite *sprite, int16 *out);
	void getSSpriteVelocity(SSprite *sprite, int16 *out);
	void getSSpriteDrag(SSprite *sprite, int16 *out);
	void setSSpriteXLimit(SSprite *sprite, int16 high, int16 low);
	void setSSpriteYLimit(SSprite *sprite, int16 high, int16 low);
	void setSSpriteInfo(SSprite *sprite, int16 info);
	int16 getSSpriteInfo(SSprite *sprite);
	void freezeSSprite(SSprite *sprite);
	void unfreezeSSprite(SSprite *sprite);

	int16 _spriteCollTempX = 0;
	int16 _spriteCollTempY = 0;
	int16 _spriteCollTempW = 0;
	int16 _spriteCollTempH = 0;
	int16 _spriteScreenX = 0;
	int16 _spriteScreenY = 0;
	int16 _spriteCollTempA[4] = { 0 };
	int16 _spriteCollTempB[4] = { 0 };
	int16 _spriteScreenAX = 0;
	int16 _spriteScreenAY = 0;
	int16 _spriteScreenBX = 0;
	int16 _spriteScreenBY = 0;

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

extern BoltEngine *g_engine;
#define SHOULD_QUIT ::Bolt::g_engine->shouldQuit()

} // End of namespace Bolt

#endif // BOLT_H
