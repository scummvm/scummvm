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

#ifndef KYRA_KYRA_V1_H
#define KYRA_KYRA_V1_H

#include "engines/engine.h"

#include "common/array.h"
#include "common/error.h"
#include "common/events.h"
#include "common/hashmap.h"
#include "common/random.h"
#include "common/rendermode.h"

#include "audio/mixer.h"

#include "kyra/script/script.h"
#include "kyra/engine/item.h"
#include "kyra/detection.h"

namespace Common {
class OutSaveFile;
class SeekableReadStream;
class WriteStream;
} // End of namespace Common

namespace Graphics {
struct Surface;
}

class KyraMetaEngine;

/**
 * This is the namespace of the Kyra engine.
 *
 * Status of this engine:
 *
 * The KYRA engine supports the following games by Westwood:
 * - Eye of the Beholder (fully supported)
 * - Eye of the Beholder II - The Legend of Darkmoon (fully supported)
 * - The Legend of Kyrandia (fully supported)
 * - The Legend of Kyrandia 2: Hand of Fate (fully supported)
 * - Lands of Lore: The Throne of Chaos (fully supported)
 * - The Legend of Kyrandia 3: Malcolm's Revenge (fully supported)
 *
 * There are various platform ports of the different games, almost all of
 * them are fully supported. We also offer legacy graphics modes like CGA
 * and EGA for games that originally supported this and also try to offer
 * all sound drivers that the originals had.
 * Some execeptions:
 * - The PC-98 version of Eye of the Beholder II is not yet supported.
 * - We don't support NES or Game Boy versions of Eye of the Beholder.
 *
 * The official translations of the games of which we are aware are mostly
 * supported. Some of the more rare versions (of which we don't even know
 * whether they are official or fan-translated) are missing. Unfortunately,
 * adding new languages is more complicated than just adding a detection
 * entry. It usually requires extra resources in kyra.dat and sometimes
 * even adjustments to the code. So it is almost impossible to support
 * translations without owning them.
 *
 * Apart from the official translations we also support an increasing number
 * of fan translations. If there is a request for a new fan translation we
 * might demand that the requesting person supplies the necessary kyra.dat
 * resource files.
 *
 */
namespace Kyra {

struct KeyCodeHash : public Common::UnaryFunction<Common::KeyCode, uint> {
	uint operator()(Common::KeyCode val) const { return (uint)val; }
};

// TODO: this is just the start of makeing the debug output of the kyra engine a bit more useable
// in the future we maybe merge some flags  and/or create new ones
enum DebugLevels {
	kDebugLevelScriptFuncs = 1 <<  0, ///< debug level for o#_* functions
	kDebugLevelScript      = 1 <<  1, ///< debug level for "EMCInterpreter" functions
	kDebugLevelSprites     = 1 <<  2, ///< debug level for "Sprites" functions
	kDebugLevelScreen      = 1 <<  3, ///< debug level for "Screen" functions
	kDebugLevelSound       = 1 <<  4, ///< debug level for "Sound" functions
	kDebugLevelAnimator    = 1 <<  5, ///< debug level for "ScreenAnimator" functions
	kDebugLevelMain        = 1 <<  6, ///< debug level for common "KyraEngine(_v#)" functions && "TextDisplayer" functions
	kDebugLevelGUI         = 1 <<  7, ///< debug level for "KyraEngine*" gui functions
	kDebugLevelSequence    = 1 <<  8, ///< debug level for "SeqPlayer" functions
	kDebugLevelMovie       = 1 <<  9, ///< debug level for movie specific funtions
	kDebugLevelTimer       = 1 << 10  ///< debug level for "TimerManager" functions
};

enum AudioResourceSet {
	kMusicIntro = 0,
	kMusicIngame,
	kMusicFinale
};

class Screen;
class Resource;
class Sound;
class TextDisplayer;
class StaticResource;
class TimerManager;
class Debugger;
class GUI;

struct Button;

class KyraEngine_v1 : public Engine {
friend class Debugger;
friend class ::KyraMetaEngine;
friend class GUI;
friend class GUI_v1;
friend class GUI_EoB;
friend class GUI_EoB_SegaCD;
friend class SoundMidiPC;		// For _eventMan
friend class SeqPlayer_HOF;		// For skipFlag()
friend class TransferPartyWiz;	// For save state API
public:
	KyraEngine_v1(OSystem *system, const GameFlags &flags);
	~KyraEngine_v1() override;

	uint8 game() const { return _flags.gameID; }
	const GameFlags &gameFlags() const { return _flags; }

	// access to Kyra specific functionallity
	Resource *resource() { return _res; }
	virtual Screen *screen() = 0;
	virtual TextDisplayer *text() { return _text; }
	virtual GUI *gui() const { return 0; }
	Sound *sound() { return _sound; }
	StaticResource *staticres() { return _staticres; }
	TimerManager *timer() { return _timer; }

	uint32 tickLength() const { return _tickLength; }

	Common::RandomSource _rnd;

	// input
	void setMousePos(int x, int y);
	Common::Point getMousePos();

	// config specific
	bool speechEnabled();
	bool textEnabled();

	enum kVolumeEntry {
		kVolumeMusic = 0,
		kVolumeSfx = 1,
		kVolumeSpeech = 2
	};

	// volume reaches per default from 2 to 97
	void setVolume(kVolumeEntry vol, uint8 value);
	uint8 getVolume(kVolumeEntry vol);

	void syncSoundSettings() override;

	// game flag handling
	int setGameFlag(int flag);
	int queryGameFlag(int flag) const;
	int resetGameFlag(int flag);

	// sound
	virtual void snd_playTheme(int file, int track);
	virtual void snd_playSoundEffect(int id, int volume=0xFF);
	virtual void snd_playWanderScoreViaMap(int command, int restart);
	virtual void snd_playVoiceFile(int id) = 0;
	virtual bool snd_voiceIsPlaying();
	virtual void snd_stopVoice();

	// delay functionallity
	virtual void delayUntil(uint32 timestamp, bool updateGameTimers = false, bool update = false, bool isMainLoop = false);
	virtual void delay(uint32 millis, bool update = false, bool isMainLoop = false);
	virtual void delayWithTicks(int ticks);

protected:
	// Engine APIs
	virtual Common::Error init();
	virtual Common::Error go() = 0;

	Common::Error run() override {
		Common::Error err;
		registerDefaultSettings();
		err = init();
		if (err.getCode() != Common::kNoError)
			return err;
		return go();
	}

	bool hasFeature(EngineFeature f) const override;
	void pauseEngineIntern(bool pause) override;

	// intern
	Resource *_res;
	Sound *_sound;
	TextDisplayer *_text;
	StaticResource *_staticres;
	TimerManager *_timer;
	EMCInterpreter *_emc;

	// input
	void setupKeyMap();
	void updateInput();
	int checkInput(Button *buttonList, bool mainLoop = false, int eventFlag = 0x8000);
	void removeInputTop();

	int _mouseX, _mouseY;

	// This is a somewhat hacky but probably least invasive way to move
	// the whole ingame screen output down a couple of lines for EOB SegaCD.
	void transposeScreenOutputY(int yAdd);
	int _transOffsY;

	struct Event {
		Common::Event event;
		bool causedSkip;

		Event() : event(), causedSkip(false) {}
		Event(Common::Event e) : event(Common::move(e)), causedSkip(false) {}
		Event(Common::Event e, bool skip) : event(Common::move(e)), causedSkip(skip) {}

		operator Common::Event() const { return event; }
	};
	Common::List<Event> _eventList;
	typedef Common::HashMap<Common::KeyCode, int16, KeyCodeHash> KeyMap;
	KeyMap _keyMap;
	bool _asciiCodeEvents;
	bool _kbEventSkip;

	// config specific
	virtual void registerDefaultSettings();
	virtual void readSettings();
	virtual void writeSettings();

	uint8 _configWalkspeed;

	int _configMusic;
	bool _configSounds;
	uint8 _configVoice;
	bool _configNullSound;

	Common::RenderMode _configRenderMode;

	// game speed
	virtual bool skipFlag() const;
	virtual void resetSkipFlag(bool removeEvent = true);

	uint16 _tickLength;
	uint16 _gameSpeed;

	// run
	int8 _deathHandler;

	// timer
	virtual void setupTimers() = 0;

	virtual void setWalkspeed(uint8 speed) = 0;

	// detection
	GameFlags _flags;

	// opcode
	virtual void setupOpcodeTable() = 0;
	Common::Array<const Opcode *> _opcodes;

	int o1_queryGameFlag(EMCState *script);
	int o1_setGameFlag(EMCState *script);
	int o1_resetGameFlag(EMCState *script);
	int o1_getRand(EMCState *script);
	int o1_hideMouse(EMCState *script);
	int o1_showMouse(EMCState *script);
	int o1_setMousePos(EMCState *script);
	int o1_setHandItem(EMCState *script);
	int o1_removeHandItem(EMCState *script);
	int o1_getMouseState(EMCState *script);
	int o1_setDeathHandler(EMCState *script);
	int o1_playWanderScoreViaMap(EMCState *script);
	int o1_fillRect(EMCState *script);
	int o1_blockInWalkableRegion(EMCState *script);
	int o1_blockOutWalkableRegion(EMCState *script);
	int o1_playSoundEffect(EMCState *script);

	// script debug
#ifndef RELEASE_BUILD
	int16 emcSafeReadStack(EMCState *s, int x, int line, const char *file) {
		if (s->sp+x > EMCState::kStackLastEntry) {
			//assert(sp+x < kStackSize);
			warning("Invalid EMC stack read attempt from: '%s', line %d", file, line);
			return 0;
		}
		return s->stack[s->sp+x];
	}
#endif

	// items
	int _mouseState;

	virtual void setHandItem(Item item) = 0;
	virtual void removeHandItem() = 0;

	// game flags
	uint8 _flagsTable[100]; // TODO: check this value

	// sound
	Audio::SoundHandle _speechHandle;

	int _curMusicTheme;
	int _curSfxFile;
	int16 _lastMusicCommand;

	const int8 *_trackMap;
	int _trackMapSize;

	bool _preventScriptSfx;

	virtual int convertVolumeToMixer(int value);
	virtual int convertVolumeFromMixer(int value);

	// pathfinder
	virtual int findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize);
	int findSubPath(int x, int y, int toX, int toY, int *moveTable, int start, int end);
	int getFacingFromPointToPoint(int x, int y, int toX, int toY);
	int getOppositeFacingDirection(int dir);
	void changePosTowardsFacing(int &x, int &y, int facing);
	int getMoveTableSize(int *moveTable);
	virtual bool lineIsPassable(int x, int y) = 0;

	static const int8 _addXPosTable[];
	static const int8 _addYPosTable[];

	// Character

	static const int8 _charAddXPosTable[];
	static const int8 _charAddYPosTable[];

	// save/load
	int _gameToLoad;

	bool _isSaveAllowed;

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override { return _isSaveAllowed; }
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override { return _isSaveAllowed; }
	int getAutosaveSlot() const override { return 999; }

	const char *getSavegameFilename(int num);
	Common::String _savegameFilename;
	static Common::String getSavegameFilename(const Common::String &target, int num);
	bool saveFileLoadable(int slot);

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;

		bool originalSave;  // savegame from original interpreter
		bool oldHeader;     // old scummvm save header

		Graphics::Surface *thumbnail;

		TimeDate timeDate;
		uint32 totalPlaySecs;
	};

	enum ReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	WARN_UNUSED_RESULT static ReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *file, SaveHeader &header, bool skipThumbnail = true);

	void loadGameStateCheck(int slot);
	Common::Error loadGameState(int slot) override = 0;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override {
		return saveGameStateIntern(slot, desc.c_str(), 0);
	}
	virtual Common::Error saveGameStateIntern(int slot, const char *saveName, const Graphics::Surface *thumbnail) = 0;

	Common::SeekableReadStream *openSaveForReading(const char *filename, SaveHeader &header, bool checkID = true);
	Common::OutSaveFile *openSaveForWriting(const char *filename, const char *saveName, const Graphics::Surface *thumbnail) const;

	// TODO: Consider moving this to Screen
	virtual Graphics::Surface *generateSaveThumbnail() const { return 0; }

	// Officially used in EOB SegaCD (appears in the final stats), but we also use this for the savegame metadata for all games.
	void updatePlayTimer();
	void restartPlayTimerAt(uint32 totalPlaySecs);
	void pausePlayTimer(bool pause);

	uint32 _lastSecTick;
	uint32 _lastSecTickAtPauseStart;
	uint32 _totalPlaySecs;
};

} // End of namespace Kyra

#endif
