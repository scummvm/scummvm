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

#ifndef IGOR_H
#define IGOR_H

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

#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"

#include "igor/detection.h"
#include "igor/resource_ids.h"

namespace Igor {

struct IgorGameDescription;

enum {
	kFlagDemo   = 1 << 0,
	kFlagFloppy = 1 << 1,
	kFlagTalkie = 1 << 2
};

enum {
	kStartupPart = 900,
	kInvalidPart = 255,
	kSharewarePart = 950,
	kTalkColor = 240,
	kTalkShadowColor = 241,
	kTickDelay = 1193180 / 4096,
	kTimerTicksCount = 8,
	kQuickSaveSlot = 0,
	kMaxSaveStates = 10,
	kNoSpeechSound = 999
};

struct DetectedGameVersion {
	int version;
	int flags;
	Common::Language language;
	const char *ovlFileName;
	const char *sfxFileName;
};

struct ResourceEntry {
	int id;
	uint32 offs;
	uint32 size;
};

struct StringEntry {
	int id;
	Common::String str;

	StringEntry() : id(0) {}
	StringEntry(int i, const char *s) : id(i), str(s) {}
};

class IgorEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	// MidiPlayer *_midiPlayer;


	Common::File _ovlFile;

	uint8 *_screenVGA;
	uint8 *_facingIgorFrames[4];
	uint8 *_screenLayer1;
	uint8 *_screenLayer2;
	uint8 *_screenTextLayer;
	uint8 *_screenTempLayer;
	uint8 *_igorHeadFrames;
	uint8 *_animFramesBuffer;
	uint8 *_inventoryPanelBuffer;
	uint8 *_inventoryImagesBuffer;
	uint8 *_verbsPanelBuffer;

	bool _eventQuitGame;
	uint32 _nextTimer;


	DetectedGameVersion _game;


	int16 _currentPart;
	uint8 _currentPalette[768];
	uint8 _paletteBuffer[768];

	int _gameTicks;
	int _resourceEntriesCount;
	int _soundOffsetsCount;
	uint32 *_soundOffsets;

	ResourceEntry *_resourceEntries;
	Common::Array<StringEntry> _stringEntries;


	void restart();
	void setupDefaultPalette();

	void readTableFile();
	void loadMainTexts();
	void loadIgorFrames();

	void PART_MAIN();
	void PART_05();

	void handleRoomInput();

	void enterPartLoop();
	void leavePartLoop();
	void runPartLoop();

	void waitForTimer(int ticks = -1);

	ResourceEntry *findData(int num);
	uint8 *loadData(int num, uint8 *dst = 0, int *size = 0);
	void loadRoomData(int pal, int img, int box, int msk, int txt);


protected:
	// Engine APIs
	Common::Error run() override;
public:
	Graphics::Screen *_screen = nullptr;
public:
	IgorEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~IgorEngine() override;

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
};

extern IgorEngine *g_engine;
#define SHOULD_QUIT ::Igor::g_engine->shouldQuit()

} // End of namespace Igor

#endif // IGOR_H
