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

#ifndef COMFY_H
#define COMFY_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "comfy/detection.h"

#define COMFY_SCREEN_WIDTH 320
#define COMFY_SCREEN_HEIGHT 200
#define COMFY_PANTHER_SCREEN_WIDTH 640
#define COMFY_PANTHER_SCREEN_HEIGHT 480
#define COMFY_PIT_INPUT_FREQUENCY 1193182
#define COMFY_PIT_TIMER_DIVISOR 0x2E9B
#define COMFY_RESOLUTION_CHANGE_CAPACITY 100
#define COMFY_PALETTE_BYTES 0x300

namespace Comfy {

class MidiPlyrDriver;

enum ComfyEngineVersion {
	kEngineVersion1, // 1994-1995
	kEngineVersion2, // 1996-1997
	kEngineVersion3  // 1999
};

class ComfyEngine : public Engine {
private:
	struct VideoRectRecord {
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
		uint16 area;
	};

	const ADGameDescription *_gameDescription;
	ComfyEngineVersion _engineVersion;
	Common::Path _gameDirectory;
	Common::Path _introDirectory;
	Common::Path _gameDataPath;
	Common::Path _languageDirectories[16];
	uint16 _language;
	bool _multiLanguage;
	Common::RandomSource _randomSource;
	Graphics::Screen *_screen;
	byte *_framebufPtr;
	byte *_presentBuffer;
	uint16 _logicalScreenWidth;
	uint16 _logicalScreenHeight;
	VideoRectRecord _resolutionChanges[COMFY_RESOLUTION_CHANGE_CAPACITY];
	uint16 _resolutionChangeCount;
	uint16 _renderDirtyCount;
	bool _renderInterleaved;
	Common::SeekableReadStream *_colorDatStream;
	byte _paletteFadeSource[COMFY_PALETTE_BYTES];
	byte _paletteTarget[COMFY_PALETTE_BYTES];
	byte _paletteDisplay[COMFY_PALETTE_BYTES];
	byte _logicalPalette[256 * 4];
	byte *_paletteDataPtr;
	uint16 _vsyncPending;
	uint16 _fadeMax;
	uint16 _fadeStep;
	byte _palettePage;
	bool _paletteFading;
	int16 _timerCurrent;
	int16 _timer0;
	int16 _timer1;
	int16 _timer2;
	uint32 _midiTimeCounter;
	uint32 _midiInstanceEventTime;
	int32 _midiEventBaseTime;
	int32 _midiTimeScale;
	int16 _midiTimeDelta;
	int16 _midiCounterAdjustment;
	MidiPlyrDriver *_midiPlyrDriver;
	uint32 _timerLastMillis;
	uint64 _pitAccumulator;
	bool _gameInitialized;
	bool _videoInitialized;
	bool _timerInitialized;
	bool _lptKeyboardInitialized;
	bool _mainLoopRunning;

	Common::Error gameInit();
	void gameShutdown();
	void gameConfigInit();
	void findLanguageDirectories();
	bool iniReadGameConfig();
	bool iniGetGameDataPath(uint16 sceneId);
	void iniWriteLanguage(uint16 language);
	Common::Path getLanguageDirectory(uint16 language);
	void pathSetGameDataDir(const Common::Path &path);
	Common::Path pathBuild(const Common::Path &filename, bool useGamePath);
	Common::SeekableReadStream *pathFOpen(const Common::Path &filename, bool useGamePath);
	void videoInit();
	void videoShutdown();
	void videoSetResolution();
	void videoFindBestMode(VideoRectRecord record);
	void videoPresentFrame();
	void renderSetDirty();
	void renderFlushDirty();
	void framebufCopyAll(byte *destination, byte *source);
	void framebufClear(uint16 color);
	uint32 framebufferBytes();
	void colorDatOpen();
	void colorDatClose();
	void colorDatReadEntry(uint16 paletteId);
	void vsyncSetPalettePtr(byte *palette);
	void paletteInterpolate(uint16 step, uint16 maximum);
	void paletteLoadWithFade(uint16 paletteId, uint16 fadeTicks);
	void paletteApplyBrightness(uint16 brightness);
	void paletteVsyncFlip();
	void paletteConvertRgbToLogical(byte *source, byte *destination);
	void paletteRealize(byte *rawPalette);
	uint16 midiTick();
	void midiSetTimeScale(int16 delta);
	int16 midiGetCounterAdjustment();
	bool midiPlyrStart();
	void midiPlyrStop();
	void timerInit();
	void timerShutdown();
	void lptKeyboardInit();
	void lptKeyboardShutdown();
	void gameMainLoop();
	void gameMainLoopTick();
	void waitForTimerTick();
	void processEvents();

	uint16 timerTick();
	void midiTrackTickAndRemove();
	void animFileTickCommands();
	void sceneTickEvent();
	void midiPollChannels(uint16 ticks);
	void paletteFadeStep(uint16 ticks);
	void lptKeyboardScanAndProcess();
	void actorTickTree();
	void renderFrame();
	void processInput();
	void processMusicEvents();
	void processSceneTransition();

protected:
	// Engine APIs
	Common::Error run() override;

public:
	ComfyEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~ComfyEngine() override;

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
};

extern ComfyEngine *g_engine;
#define SHOULD_QUIT ::Comfy::g_engine->shouldQuit()

} // End of namespace Comfy

#endif // COMFY_H
