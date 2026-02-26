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
	BOLTCallback *typeLoadCallbacks;   // +0x00: type-based post-load fixup (25 entries)
	BOLTCallback *typeFreeCallbacks;   // +0x04: type-based post-free cleanup (25 entries)
	BOLTCallback *memberLoadCallbacks; // +0x08: per-member pre-load callbacks
	BOLTCallback *memberFreeCallbacks; // +0x0C: per-member pre-free callbacks
	BOLTCallback *groupLoadCallbacks;  // +0x10: per-group load callbacks
	BOLTCallback *groupFreeCallbacks;  // +0x14: per-group free callbacks

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
	byte flags;          // +0x00: bit 3 = uncompressed
	byte preLoadCbIndex; // +0x01: index into lib->userData1 callback table
	byte preFreeCbIndex; // +0x02: index into lib->userData2 callback table
	byte typeCbIndex;    // +0x03: index into lib->loadCallbacks (post-load)
						 //        and lib->freeCallbacks (post-free)
	uint32 decompSize;   // +0x04: decompressed data size (big-endian in file)
	uint32 fileOffset;   // +0x08: file offset of compressed/raw data (big-endian in file)
	uint32 dataPtrPlaceholder;
	byte *dataPtr; // +0x0C: pointer to loaded data (NULL if not loaded)

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
} BOLTMemberEntry; // 0x10 = 16 bytes

typedef struct BOLTGroupEntry {
	byte flags;                     // +0x00: unknown flags
	byte loadCbIndex;               // +0x01: group load callback index (into lib+0x18)
	byte freeCbIndex;               // +0x02: group free callback index (into lib+0x1C)
	byte memberCount;               // +0x03: number of members (0 = 256)
	uint32 memberDirOffset;         // +0x04: file offset to member directory (big-endian)
	uint32 memberDataOffset;        // +0x08: file offset to member data (big-endian)
	uint32 groupDataPtrPlaceholder; // +0x0C: pointer to loaded group data block (NULL if not loaded)
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

} BOLTGroupEntry;            // 0x10 = 16 bytes

typedef struct BOLTLib {
	int16 refCount;           // +0x00
	int16 groupCount;         // +0x02 (0 = 256)
	Common::File *fileHandle; // +0x04
	BOLTCallbacks callbacks;
	BOLTGroupEntry *groups; // +0x20

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
	uint32 tag;       // +0x00: chunk tag (e.g. MKTAG('A','2','2','8'))
	uint32 allocSize; // +0x04: total allocated size (including header)
	uint32 dataSize;  // +0x08: raw data size
	byte *dataPtr;    // +0x0C: pointer to data (= this + 0x1C)
	int16 skipCount;  // +0x10: how late this frame is
	int16 frameRate;    // +0x12: frame rate reference (or 50 if no sound)
	int16 duration;   // +0x14: sound duration in hundredths of a second
	int16 timestamp;  // +0x16: cumulative timeline position
	RTFPacket *next;  // +0x18: next packet in linked list
	// +0x1C: data begins here
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
} RTFPacket; // 0x1C header = 28 bytes

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
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
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

	// xpMain
	void boltMain();
	void setCursorPict(byte *sprite);

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
	int16 openBooth(int16 sceneId);
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
	void mapIdleAnimation();
	void boothIdleAnimation();
	void screensaverStep();
	void fadeToBlack(int16 steps);
	void flushInput();
	int16 winALetter(int16 prevBooth);

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
	byte *g_boothExitLeft = nullptr;
	byte *g_boothExitRight = nullptr;
	byte g_leftDoorNavTable[3] = {3, 2, 4};
	byte g_rightDoorNavTable[3] = {1, 0, 5};

	int16 g_cursorX = 0;
	int16 g_cursorY = 0;
	int16 g_hoveredHotspot = 0;
	uint32 g_keyTimer = 0;
	int16 g_keyReleased = 0;
	int16 g_keyLeft = 0;
	int16 g_keyRight = 0;
	int16 g_keyUp = 0;
	int16 g_keyDown = 0;
	int16 g_keyHelp = 0;
	int16 g_keyEnter = 0;

	int16 g_screensaverStep = 0;
	int16 g_helpPlaying = 0;
	int16 g_helpIsIdle = 0;
	int16 g_idleHelpAvailable = 0;

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
	void sub_11035();
	void boltPict2Pict(XPPicDesc *dest, byte *boltSprite);
	void displayPic(byte *boltSprite, int16 xOff, int16 yOff, int16 page);
	void sub_11131();
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
	BOLTCallbacks g_resourceDefaultCallbacks;

	static BOLTCallback g_defaultTypeLoadCallbacks[25];
	static BOLTCallback g_defaultTypeFreeCallbacks[25];
	static BOLTCallback g_defaultMemberLoadCallbacks[25];
	static BOLTCallback g_defaultMemberFreeCallbacks[25];
	static BOLTCallback g_defaultGroupLoadCallbacks[25];
	static BOLTCallback g_defaultGroupFreeCallbacks[25];

	static void noOpCb();

	static void swapAllWordsCb();
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

	// Game state
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

	// --- MINIGAMES ---

	// Fred
	int16 fredGame(int16 prevBooth) { return 0; }

	// George
	int16 georgeGame(int16 prevBooth) { return 0; }

	// Huck
	int16 huckGame(int16 prevBooth) { return 0; }

	// Scooby
	int16 scoobyGame(int16 prevBooth) { return 0; }

	// TopCat
	int16 topCatGame(int16 prevBooth) { return 0; }

	// Yogi
	int16 yogiGame(int16 prevBooth) { return 0; }
};

extern BoltEngine *g_engine;
#define SHOULD_QUIT ::Bolt::g_engine->shouldQuit()

} // End of namespace Bolt

#endif // BOLT_H
