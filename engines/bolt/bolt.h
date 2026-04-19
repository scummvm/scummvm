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

class BoltEngine : public Engine {
friend class XpLib;

private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
protected:
	// BoltEngine can only be constructed by one of the Carnival, Merlin or Crete engines.
	BoltEngine(OSystem *syst, const ADGameDescription *gameDesc);

	// Engine APIs
	Common::Error run() override;

public:
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
	virtual void boltMain() = 0;
	void setCursorPict(byte *sprite);

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

	virtual void initCallbacks() = 0;

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
	int32 _curErrorCode = 0;

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
};

extern BoltEngine *g_engine;
#define SHOULD_QUIT ::Bolt::g_engine->shouldQuit()

} // End of namespace Bolt

#endif // BOLT_H
