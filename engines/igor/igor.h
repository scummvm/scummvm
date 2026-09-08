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

#include "common/error.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
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
	kFlagDemo = 1 << 0,
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

enum {
	MAX_DIALOGUE_TEXTS = 6,
	MAX_OBJECT_NAME_LENGTH = 31,
	MAX_DIALOGUE_TEXT_LENGTH = 51,
	MAX_VERB_NAME_LENGTH = 12,
	MAX_ROOM_OBJECT_AREAS = 256,
	MAX_DIALOGUE_QUESTIONS = 30,
	MAX_DIALOGUE_REPLIES = 70
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

struct RoomObjectArea {
	uint8 area;
	uint8 object;
	uint8 y1Lum;
	uint8 y2Lum;
	uint8 deltaLum;
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
	int _screenVGAVOffset;

	bool _eventQuitGame;
	uint32 _nextTimer;

	DetectedGameVersion _game;


	char _globalDialogueTexts[300][MAX_DIALOGUE_TEXT_LENGTH];
	char _roomObjectNames[20][MAX_OBJECT_NAME_LENGTH];
	uint8 _walkXScaleRoom[320];
	uint8 _walkYScaleRoom[144 * 3];
	RoomObjectArea _roomObjectAreasTable[MAX_ROOM_OBJECT_AREAS];

	int16 _currentPart;
	uint8 _currentPalette[768];
	uint8 _paletteBuffer[768];
	uint8 _igorPalette[48];

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
	void PART_05_UPDATE_ROOM_BACKGROUND();

	void handleRoomInput();

	void enterPartLoop();
	void leavePartLoop();
	void runPartLoop();


	void scrollPalette(int startColor, int endColor);
	void setPaletteColor(uint8 index, uint8 r, uint8 g, uint8 b);
	void setPaletteRange(int startColor, int endColor);
	void updatePalette(int count);

	void fadeInPalette(int count);
	void fadeOutPalette(int count);

	void decodeRoomStrings(const uint8 *p, bool skipObjectNames = false);
	void decodeRoomText(const uint8 *p);
	void decodeRoomAreas(const uint8 *p, int count);
	void decodeRoomMask(const uint8 *p);

	bool compareGameTick(int add, int mod) const { return ((_gameTicks + (add & ~7)) % mod) == 0; } // { return ((_gameTicks + add) % mod) == 0; }
	bool compareGameTick(int eq) const { return _gameTicks == (eq & ~7); } // { return _gameTicks == eq; }

	void waitForTimer(int ticks = -1);

	ResourceEntry *findData(int num);
	uint8 *loadData(int num, uint8 *dst = 0, int *size = 0);
	void loadRoomData(int pal, int img, int box, int msk, int txt);

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
		return (f == kSupportsLoadingDuringRuntime) ||
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
	// Engine APIs
	Common::Error run() override;
	static const uint8 PAL_48_1[];
	static const uint8 PAL_96_1[];
	static const uint8 PAL_IGOR_1[];
	static const uint8 PAL_IGOR_2[];
};

extern IgorEngine *g_engine;
#define SHOULD_QUIT ::Igor::g_engine->shouldQuit()

} // End of namespace Igor

#endif // IGOR_H
