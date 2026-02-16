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

struct BoltGameDescription;
class XpLib;

struct DisplaySpecs {
	int16 width;
	int16 height;
	int16 depth;
	int16 unk;
};

struct BarkerTable {
	
};

struct BOLTLib {
	int16 refCount;
	int16 groupCount;
	int32 fileHandle;
	int32 *funcTable1;
	int32 *funcTable2;
	int32 *dwordPtr1;
	int32 *dwordPtr2;
	int32 *dwordPtr3;
	int32 *dwordPtr4;
};

class BoltEngine : public Engine {
friend class XpLib;

private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
protected:
	// Engine APIs
	Common::Error run() override;
public:
	Graphics::Screen *_screen = nullptr;
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
		{384, 240, 1, 1},
		{320, 200, 1, 1}
	};

	XpLib *_xp;

	// Entry point
	void boltMain();

	// Booths logic
	typedef void (BoltEngine::*SideShowHandler)(void);

	int g_lettersWon = 0;

	void displayBooth();
	void playAVOverBooth();
	void hucksBooth();
	void fredsBooth();
	void scoobysBooth();
	void yogisBooth();
	void georgesBooth();
	void topCatsBooth();
	void mainEntrance();
	void huckGame();
	void fredGame();
	void scoobyGame();
	void yogiGame();
	void georgeGame();
	void topCatGame();
	void winALetter();

	void loadBooth();
	void unloadBooth();
	void openBooth();
	void closeBooth();
	void playTour();
	void finishPlayingHelp();
	void hotSpotActive();
	void hoverHotSpot();
	void boothEventLoop();
	void resetInactivityState();
	void handleButtonPress();
	void playBoothAV();
	void mapIdleAnimation();
	void boothIdleAnimation();
	void screensaverStep();
	void flushInput();
	
	BarkerTable *createBarker(int16 minIndex, int16 maxIndex);
	void freeBarker(BarkerTable *table);
	void registerSideShow(BarkerTable *table, SideShowHandler handler, short boothId);
	void barker(BarkerTable *table, int16 startBooth);
	bool checkError();

	// Graphics
	int g_displayMode = 0;
	int32 g_displayX = 0;
	int32 g_displayY = 0;
	int32 g_displayWidth = 0;
	int32 g_displayHeight = 0;

	void setCursorPict(void *sprite);
	void startCycle(void *cycleResource);
	void blastColors();
	void setColors();
	void restoreColors();
	void loadColors();
	void shiftColorMap();
	void fadeToBlack();
	void displayColors(void *palette, int16 page, int16 flags);
	void boltPict2Pict(void *dest, void *boltSprite);
	void displayPic(void *boltSprite, int16 xOff, int16 yOff, int16 flags);
	void boltCycleToXPCycle();
	void unpackColors();
	void inRect();
	void rectOverlap();

	// Resource handling
	BOLTLib *g_boothsBoltLib = nullptr;
	int g_boothsBoltIndex = 0;

	#define AssetPath(x) x

	bool openBOLTLib(const char *fileName, int *outIdx, BOLTLib **outLib);
	void closeBOLTLib(BOLTLib **lib);
	bool attemptFreeIndex(BOLTLib *lib, int16 groupId);
	void loadGroupDirectory();
	bool getBOLTGroup(BOLTLib *lib, int16 groupId, int16 flags);
	void freeBOLTGroup(BOLTLib *lib, int16 groupId, int16 flags);
	void *getBOLTMember(BOLTLib *lib, int16 resId);
	bool freeBOLTMember(BOLTLib *lib, int16 resId);

	void *memberAddr(BOLTLib *lib, int16 resId);
	void *memberAddrOffset(BOLTLib *lib, uint32 resIdAndOffset);
	uint32 memberSize(BOLTLib *lib, int16 resId);
	void *groupAddr(BOLTLib *lib, int16 groupId);

	bool allocResourceIndex();
	void freeResourceIndex();

	bool initVRam(int16 poolSize);
	void freeVRam();
	bool vLoad(void *dest, const char *name);
	bool vSave(void *src, uint16 srcSize, const char *name);
	bool vDelete(const char *name);
	void memMove(void *dest, void *src, uint16 count);
	void *dataAddress(int16 recordOffset);
	uint16 dataSize(int16 recordOffset);
	bool findRecord(const char *name, int16 *outOffset);

	// Videos
	void *g_rtfHandle = nullptr;

	void *openRTF(const char *fileName);
	void closeRTF(void *rtf);
	bool playRTF(void *rtfFile, int16 animIndex, void *ringBuffer, int32 bufferSize);
	bool fillRTFBuffer();
	void flushRTFSoundQueue();
	bool maintainRTF(int16 mode, void *outFrameData);
	bool isRTFPlaying();
	void killRTF();
	void readPacket();
	void preProcessPacket();
	void queuePacket();
	void deQueuePacket();
	void allocPacket();
	void freePacket();
	void resetPlaybackState();
	void sub_12980();
	void prepareAV();
	void maintainAV();
	void stopAV();
	bool playAV(void *rtfHandle, int16 animIndex, int16 width, int16 height, int16 xOff, int16 yOff);
	void processPacket();
	void processRL7();
	void processPLTE();
	void initAV();
	void cleanUpAV();
	void startAnimation();
	void maintainAudioPlay();
	void initAnim();
	void cleanUpAnim();
};

extern BoltEngine *g_engine;
#define SHOULD_QUIT ::Bolt::g_engine->shouldQuit()

} // End of namespace Bolt

#endif // BOLT_H
