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

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return false;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return false;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

protected:
	DisplaySpecs g_displaySpecs[2] = {
		{0, 384, 240},
		{1, 320, 200}
	};

	XpLib *_xp = nullptr;
	bool g_extendedViewport = false;
	bool g_isDemo = false;

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

	int16 g_lettersWon = 0;
	bool g_allLettersWonFlag = false;
	int g_displayMode = 0;
	int32 g_displayX = 0;
	int32 g_displayY = 0;
	int32 g_displayWidth = 0;
	int32 g_displayHeight = 0;

	int16 g_boothLoadedMask = 0;
	int16 g_currentBoothScene = 0;
	int16 g_boothNumHotspots = 0;
	int16 g_boothNumAnimations = 0;
	byte *g_boothSceneDesc = nullptr;
	byte *g_boothHotPalDescs[4];
	Common::Rect g_boothHotspotDescs[8]; 
	byte *g_boothAnimDescs[7];
	byte *g_boothAnimPalDescs[4];
	byte g_savedPalettes[7][30];
	byte g_savedHotPalettes[4][9];
	byte *g_boothPalCycleData = nullptr;
	XPPicDesc g_boothLetterSprite;
	bool g_needInitCursorPos = true;
	byte *g_boothVisitSignOn = nullptr;
	byte *g_boothVisitSignOff = nullptr;
	byte g_leftDoorNavTable[3] = {3, 2, 4};
	byte g_rightDoorNavTable[3] = {1, 0, 5};

	int16 g_cursorX = 0;
	int16 g_cursorY = 0;
	int16 g_hoveredHotspot = 0;
	uint32 g_helpTimer = 0;
	int16 g_keyReleased = 0;
	int16 g_keyLeft = 0;
	int16 g_keyRight = 0;
	int16 g_keyUp = 0;
	int16 g_keyDown = 0;
	int16 g_helpFlag = 0;
	int16 g_keyEnter = 0;

	int16 g_tourStep = 0;
	int16 g_helpPlaying = 0;
	int16 g_helpIsIdle = 0;
	int16 g_idleHelpAudioAvailable = 1;

	int16 g_huckWins = 0;
	int16 g_fredWins = 0;
	int16 g_scoobyWins = 0;
	int16 g_yogiWins = 0;
	int16 g_georgeWins = 0;
	int16 g_topCatWins = 0;

	// Barker
	BarkerTable *createBarker(int16 minIndex, int16 maxIndex);
	void freeBarker(BarkerTable *table);
	bool registerSideShow(BarkerTable *table, SideShowHandler handler, int16 boothId);
	int16 barker(BarkerTable *table, int16 startBooth);
	bool checkError();

	int32 g_curErrorCode = 0;
	int16 g_currentBoothId = 0;

	// Utils
	void displayColors(byte *palette, int16 page, int16 flags);
	byte getPixel(byte *sprite, int16 localX, int16 localY);
	void boltPict2Pict(XPPicDesc *dest, byte *boltSprite);
	void displayPic(byte *boltSprite, int16 xOff, int16 yOff, int16 page);
	bool pointInRect(Common::Rect *rect, int16 x, int16 y);
	const char *assetPath(const char *fileName);
	void boltCycleToXPCycle(byte *srcData, XPCycleState *cycleDesc);
	void unpackColors(int16 count, byte *packedColors);

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

	BOLTLib *g_boothsBoltLib = nullptr;
	BOLTCallbacks g_boothsBoltCallbacks;

	static BOLTCallback g_defaultTypeLoadCallbacks[25];
	static BOLTCallback g_defaultTypeFreeCallbacks[25];
	static BOLTCallback g_defaultMemberLoadCallbacks[25];
	static BOLTCallback g_defaultMemberFreeCallbacks[25];
	static BOLTCallback g_defaultGroupLoadCallbacks[25];
	static BOLTCallback g_defaultGroupFreeCallbacks[25];

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

	int16 g_resourceIndexCount = 1000;
	uint32 **g_resourceIndex = nullptr;
	Common::Array<byte *> g_resolvedPtrs;
	Common::File *g_cachedFileHandle = nullptr;
	uint32 g_cachedFilePos = 0xFFFFFFFF;
	BOLTLib *g_boltCurrentLib = nullptr;
	BOLTHeader g_boltFileHeader;
	BOLTGroupEntry *g_boltCurrentGroupEntry = nullptr;
	int16 g_boltLoadDepth = 0;
	byte *g_boltRawMemberData = nullptr;
	BOLTMemberEntry *g_boltCurrentMemberEntry = nullptr;
	int16 g_pendingFixupCount = 0;

	// Game levelNumber
	bool initVRam(int16 poolSize);
	void freeVRam();
	bool vLoad(void *dest, const char *name);
	bool vSave(void *src, uint16 srcSize, const char *name);
	bool vDelete(const char *name);
	byte *dataAddress(int16 recordOffset);
	uint16 dataSize(int16 recordOffset);
	bool findRecord(const char *name, int16 *outOffset);

	int32 g_vramRecordCount = 0;
	int32 g_vramUsedBytes = 0;
	byte *g_allocatedMemPool = nullptr;
	uint32 g_allocatedMemPoolSize = 0;

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

	RTFResource *g_rtfHandle = nullptr;
	Common::File *g_rtfFileHandle = nullptr;
	uint32 g_rtfChunkRemaining = 0;
	bool g_rtfMidChunk = false;
	RTFPacket *g_rtfCurrentPacket = nullptr;
	byte *g_ringBufBase = nullptr;
	byte *g_ringBufWritePtr = nullptr;
	uint32 g_ringBufSize = 0;
	uint32 g_ringBufFreeSpace = 0;
	uint32 g_ringBufLowWater = 0;
	uint32 g_ringBufHighWater = 0;
	uint32 g_ringBufUsed = 0;
	bool g_rtfSoundActive = false;
	int16 g_rtfPlaybackTime = 0;
	int16 g_rtfCumulativeTime = 0;
	RTFPacket *g_rtfPendingFrame = nullptr;
	RTFPacket *g_rtfSoundQueueHead = nullptr;
	RTFPacket *g_rtfSoundPlayHead = nullptr;
	RTFPacket *g_rtfChunkListTail = nullptr;
	RTFPacket *g_rtfChunkListHead = nullptr;
	int16 g_rtfChunkCount = 0;
	int16 g_rtfQueuedSoundCount = 0;
	int16 g_rtfSoundTiming = 0;
	uint32 g_rtfAnimStartOffset = 0;
	bool g_rtfNeedInitialFill = false;
	uint32 g_rtfChunkTag = 0;
	uint32 g_rtfChunkSize = 0;

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

	byte *g_avRingBuffer = nullptr;
	uint32 g_avTargetBufSize = 0x0FA000;
	int16 g_avSkipLevel = 0;
	byte *g_avFrontPalette = nullptr;
	byte *g_avBackPalette = nullptr;
	uint32 g_avSavedInactivityTimer = 0;
	uint32 g_avSavedScreenSaverTimer = 0;
	int16 g_avFrameAccum = 0;
	int16 g_avDisplayX = 0;
	int16 g_avDisplayY = 0;

	XPPicDesc g_avFrontBufDesc;
	XPPicDesc g_avBackBufDesc;

	// Anim
	bool startAnimation(RTFResource *rtf, int16 animIndex);
	void stopAnimation();
	bool maintainAudioPlay(int16 mode);
	bool initAnim(RTFResource *rtf, int16 animIndex);
	void cleanUpAnim();

	int16 g_animPrevInactivityTimer = 0;
	byte *g_animRingBuffer = nullptr;
	Common::File *g_animFileHandle = nullptr;

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

	int16 g_spriteCollTempX = 0;
	int16 g_spriteCollTempY = 0;
	int16 g_spriteCollTempW = 0;
	int16 g_spriteCollTempH = 0;
	int16 g_spriteScreenX = 0;
	int16 g_spriteScreenY = 0;
	int16 g_spriteCollTempA[4] = { 0 };
	int16 g_spriteCollTempB[4] = { 0 };
	int16 g_spriteScreenAX = 0;
	int16 g_spriteScreenAY = 0;
	int16 g_spriteScreenBX = 0;
	int16 g_spriteScreenBY = 0;

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

	BOLTLib *g_fredBoltLib = nullptr;
	BOLTCallbacks g_fredBoltCallbacks;

	static BOLTCallback g_fredTypeLoadCallbacks[28];
	static BOLTCallback g_fredTypeFreeCallbacks[28];

	byte *g_fredLevelPtr = nullptr;
	byte *g_fredBackground = nullptr;
	byte *g_fredBalloonString = nullptr;
	byte *g_fredPalette = nullptr;
	byte *g_fredFacingLeftRect = nullptr;
	byte *g_fredFacingRightRect = nullptr;
	byte *g_fredTurningRect = nullptr;
	byte *g_fredBalloonRect = nullptr;
	byte *g_fredHelpEntries = nullptr;
	byte *g_fredPlayButton = nullptr;
	byte *g_fredSprites[10] = { nullptr };
	FredEntityState **g_fredEntitiesTable = nullptr;

	byte *g_fredRowBounds = nullptr;
	byte *g_fredShuffleTable = nullptr;
	byte *g_fredCycleRaw = nullptr;
	byte *g_fredBalloonSprite = nullptr;
	byte *g_fredPathMatrix = nullptr;
	byte *g_fredCurrentHelpObject = nullptr;
	byte *g_fredHoveredEntry = nullptr;

	SoundInfo g_fredSounds[4];

	SoundInfo *g_fredCurrentSound = nullptr;
	SoundInfo *g_fredLoopSound = nullptr;
	SoundInfo *g_fredPendingOneShot = nullptr;
	SoundInfo *g_fredPendingLoop = nullptr;

	FredEntityState g_fredSprite;

	int16 g_fredSaveData[3] = { 0, 0, 0 };
	int16 g_fredLevelIndex = 0;

	int16 g_fredBalloonSpawnDelay = 0;
	int16 g_fredBalloonSearchIdx = 0;
	uint32 g_fredTimer = 0;
	int16 g_fredHelpStep = -1;
	int16 g_fredShowHelp = 1;

	XPCycleState g_fredCycleSpecs[4];

	const char *g_fredSaveFile = "FredBC";

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

	BOLTLib *g_georgeBoltLib = nullptr;
	BOLTCallbacks g_georgeBoltCallbacks;

	static BOLTCallback g_georgeTypeLoadCallbacks[28];
	static BOLTCallback g_georgeTypeFreeCallbacks[28];

	byte *g_georgePrevActiveHelpObject = nullptr;
	byte *g_georgeActiveHelpObject = nullptr;
	int16 g_georgeHelpStep = -1;
	int16 g_georgeHelpActive = 1;
	const char *g_georgeSaveFileName = "GeorgeBE";

	GeorgeEntityState **g_georgeEntityList = nullptr;
	int16 g_georgeCarIdx = 0;
	int16 g_georgeNumSatellites = 0;
	int16 g_georgeFirstAsteroidIdx = 0;
	int16 g_georgeTotalSatellites = 0;
	byte *g_georgeCarPics[3] = { nullptr };
	byte *g_georgeHelpObjects = nullptr;
	byte *g_georgeHelpSequence = nullptr;
	uint32 g_georgeHelpTimer = 0;
	byte *g_georgeSatelliteShuffleTable = nullptr;
	byte *g_georgeAsteroidShuffleTable = nullptr;
	int16 *g_georgeSatelliteThresholds = nullptr;
	int16 *g_georgeAsteroidThresholds = nullptr;
	byte *g_georgeSatellitePaths = nullptr;
	byte *g_georgeAsteroidPaths = nullptr;
	byte *g_georgeCollisionRect = nullptr;
	byte *g_georgeSatelliteCollisionRects = nullptr;
	byte *g_georgeAsteroidCollisionRects = nullptr;
	byte *g_georgeSatelliteGfx = nullptr;
	byte *g_georgeAsteroidGfx = nullptr;
	int16 g_georgeSatelliteWait = 0;
	int16 g_georgeAsteroidWait = 0;
	int16 g_georgeSatelliteSearchIdx = 0;
	int16 g_georgeAsteroidSearchIdx = 0;
	int16 g_georgeHitSearchIdx = 0;
	byte *g_georgeBgPic = nullptr;
	byte *g_georgePalette = nullptr;
	byte *g_georgePalCycleRawData = nullptr;
	XPCycleState g_georgePalCycleSpecs[4];
	int16 g_georgeCollectedSatellitesNum = 0;
	SoundInfo g_georgeSoundCarStartUp;
	SoundInfo g_georgeSoundCarGoesAway;
	SoundInfo g_georgeSoundCarLoopHi;
	SoundInfo g_georgeSoundCarLoopLo;
	SoundInfo g_georgeSoundCarTumble;
	SoundInfo **g_georgeSatelliteSoundList = nullptr;
	byte g_georgeSoundChannelCounter = 0;
	SoundInfo *g_georgeSoundToPlay = nullptr;
	SoundInfo *g_georgeSoundCurrent = nullptr;
	SoundInfo *g_georgeSoundQueued = nullptr;
	SoundInfo *g_georgeSoundNext = nullptr;
	int16 g_georgeSaveData[3] = { 0 };
	int16 *g_georgeThresholds = nullptr;

	// --- HUCK ---
	void playSoundMapHuck(int16 memberId);
	void waitSoundMapHuck();
	void setHuckColors(int16 which);
	void restoreHuckColors(int16 which);
	void startHuckShuffleTimer();
	bool intersectRect(const Common::Rect *a, const Common::Rect *b, Common::Rect *out);
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

	BOLTLib *g_huckBoltLib = nullptr;
	BOLTCallbacks g_huckBoltCallbacks;

	static BOLTCallback g_huckTypeLoadCallbacks[27];
	static BOLTCallback g_huckTypeFreeCallbacks[27];

	int16 g_huckSoundPlaying = 0;
	int16 g_huckHotSpotCount = 0;
	int16 g_huckActionState = 0;
	uint32 g_huckScreensaverTimer = 0;
	uint32 g_huckBlinkTimer = 0;
	int16 g_huckScreensaverFlag = 0;
	int16 g_huckBlinkFlag = 0;
	uint32 g_huckShuffleTimer = 0;
	int16 g_huckGlobal[30] = { 0 };
	HuckState g_huckState;
	byte *g_huckGiftPic = nullptr;
	byte *g_huckBgPic = nullptr;
	int16 g_huckGiftGroupId = 0;
	int16 g_huckVariantGroupId = 0;
	byte *g_huckBgDisplayPic = nullptr;
	int16 g_huckScrollOffset = 0;
	int16 g_huckPalRange[8] = { 0 };
	int16 g_returnBooth = 0;
	int16 g_huckExitFlag = 0;
	int16 g_huckCursorX = 0;
	int16 g_huckCursorY = 0;
	byte g_huckPalSave0[15] = { 0 };
	byte g_huckPalHighlight0[15] = { 0 };
	byte g_huckPalSave1[15] = { 0 };
	byte g_huckPalHighlight1[15] = { 0 };
	XPPicDesc g_huckScratchPic;

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

	BOLTLib *g_scoobyBoltLib = nullptr;
	BOLTCallbacks g_scoobyBoltCallbacks;

	static BOLTCallback g_scoobyTypeLoadCallbacks[28];
	static BOLTCallback g_scoobyTypeFreeCallbacks[28];

	int16 g_scoobySoundMode = 0;
	int16 g_scoobySoundPlaying = 0;
	int16 g_scoobyShowHelp = 1;

	int16 g_scoobyLastInputDir = 0;
	int16 g_scoobyTransitionFrom = 0;
	int16 g_scoobyTransitionTarget = 0;
	int16 g_scoobyTransitionVelX = 0;
	int16 g_scoobyTransitionVelY = 0;
	XPPicDesc g_scoobyTempPic;
	byte g_scoobyGlobalSaveData[0x3C] = { 0 };
	ScoobyState g_scoobyGameState;
	SSprite g_scoobySprite;
	byte *g_scoobyBaseData = nullptr;
	byte *g_scoobyBgPic = nullptr;
	byte *g_scoobyWallPicsA[4] = { nullptr };
	byte *g_scoobyWallPicsB[5] = { nullptr };
	ScoobyRect g_scoobyCellBounds[25];
	Common::Point g_scoobyLevelStartXY[25];
	byte *g_scoobyLevelData = nullptr;
	int16 g_scoobySelectedGraphicsGroup = 0;
	int16 g_scoobyDifficulty = 0;
	int16 g_scoobyLevelCount = 0;
	int16 g_scoobyMoveRequested = 0;
	int16 g_scoobyTransitioning = 0;
	int16 g_scoobyDesiredDir = 0;
	int16 g_scoobyInputHoldCount = 0;
	int16 g_scoobyWallAnimating = 0;
	int16 g_scoobyWallAnimStep = 0;
	int16 g_scoobyWallsToOpen = 0;
	int16 g_scoobyWallsToClose = 0;

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

	RTFResource *g_topCatRtfHandle = nullptr;
	RTFResource *g_topCatAvHandle = nullptr;
	BOLTLib *g_topCatBoltLib = nullptr;
	BOLTCallbacks g_topCatBoltCallbacks;

	static BOLTCallback g_topCatTypeLoadCallbacks[26];
	static BOLTCallback g_topCatTypeFreeCallbacks[26];

	byte *g_topCatBackgroundPalette = nullptr;
	byte *g_topCatBackground = nullptr;
	byte *g_topCatBackgroundAnimationPalette = nullptr;
	byte *g_topCatGraphicsAssets = nullptr;

	int16 g_topCatBackgroundAnimFrame = 0;
	int16 g_topCatMaxBackgroundAnimFrames = 0;
	int16 g_topCatCurrentAnimType = 0;
	int16 g_topCatAnimStateMachineStep = 0;
	int16 g_topCatAnimQueueSize = 0;
	int16 g_topCatQueuedSoundFrames = 0;
	byte *g_topCatButtonsPalette = nullptr;
	byte *g_topCatBlinkEntries = nullptr;
	byte *g_topCatLightsPalette = nullptr;
	int16 *g_topCatBlinkSeqPtr = nullptr;
	byte *g_topCatSelectedChoiceOverlayGfx = nullptr;
	byte *g_topCatCycleData = nullptr;
	XPCycleState g_topCatCycleSpecs[4];
	byte *g_topCatShuffledQuestions = nullptr;
	byte *g_topCatShuffledAnswers = nullptr;
	byte *g_topCatAnswersPermutations = nullptr;
	byte *g_topCatAnswers = nullptr;
	byte *g_topCatAnswersScreenPositions = nullptr;
	int16 g_topCatSavedScore = 0;
	int16 g_topCatSaveHistory = 0;
	const char *g_topCatSaveFileName = "TopCatBF";
	const char *g_topCatStaticSaveFileName = "TopCatBFStatic";
	int16 g_topCatScore = 0;
	int16 g_topCatShuffledQuestionsArrayIdx = 0;
	uint32 g_topCatBlinkTimer = 0;
	SoundInfo g_topCatSoundInfo;
	byte g_topCatSavedShuffledQuestions[60] = { 0 };
	byte g_topCatSavedShuffledAnswers[60] = { 0 };
	byte g_topCatSavedAnswersPermutations[60 * 3] = { 0 };
	byte g_topCatSaveBuffer[302] = { 0 };
	byte *g_topCatHoveredEntry = nullptr;
	byte *g_topCatHelpButton = nullptr;
	byte *g_topCatBackButton = nullptr;
	byte *g_topCatBlinkEntry = nullptr;
	int16 g_topCatCycleStep = 0;
	int16 g_topCatDrawnQuestionId = 0;
	int16 g_topCatCurrentAnswerIdx = 0;
	XPCycleState g_topCatChoiceCycleState[4];
	TopCatAnim g_topCatAnimQueue[3];
	int16 g_topCatCorrectAnimIdx = 0;
	int16 g_topCatWrongAnimIdx = 0;
	int16 g_topCatShouldPlayAnswerAnim = 0;
	int16 g_topCatMaintainSoundFlag = 0;
	int16 g_topCatPermTableA[3] = { 0, 1, 2 };
	int16 g_topCatPermTableB[3] = { 0, 1, 2 };
	int16 g_topCatBlinkSeq0[5] = { 0x01, 0x00, 0x01, 0x00, 0x01 };
	int16 g_topCatBlinkSeq1[5] = { 0x03, 0x01, 0x03, 0x01, 0x03 };
	int16 g_topCatBlinkSeq2[5] = { 0x07, 0x03, 0x07, 0x03, 0x07 };
	int16 g_topCatBlinkSeq3[5] = { 0x0F, 0x07, 0x0F, 0x07, 0x0F };
	int16 g_topCatBlinkSeq4[5] = { 0x1F, 0x0F, 0x1F, 0x0F, 0x1F };
	int16 g_topCatBlinkSeq5[5] = { 0x3F, 0x1F, 0x3F, 0x1F, 0x3F };
	int16 g_topCatBlinkSeq6[25] = {
		0x09, 0x12, 0x24, 0x09, 0x12, 0x24,
		0x09, 0x12, 0x24, 0x09, 0x12, 0x24,
		0x09, 0x12, 0x24, 0x09, 0x12, 0x24,
		0x09, 0x12, 0x24, 0x09, 0x12, 0x24,
		0x3F
	};

	int16 *g_topCatBlinkSeqTable[7] = {
		g_topCatBlinkSeq0, g_topCatBlinkSeq1, g_topCatBlinkSeq2,
		g_topCatBlinkSeq3, g_topCatBlinkSeq4, g_topCatBlinkSeq5,
		g_topCatBlinkSeq6
	};

	// --- YOGI ---
	int16 yogiGame(int16 prevBooth) { return 0; }
};

extern BoltEngine *g_engine;
#define SHOULD_QUIT ::Bolt::g_engine->shouldQuit()

} // End of namespace Bolt

#endif // BOLT_H
