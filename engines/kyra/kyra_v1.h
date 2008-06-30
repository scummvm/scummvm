/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_KYRA_V1_H
#define KYRA_KYRA_V1_H

#include "engines/engine.h"

#include "common/array.h"
#include "common/events.h"

#include "kyra/script.h"

namespace Common {
class InSaveFile;
class OutSaveFile;
} // end of namespace Common

class KyraMetaEngine;

namespace Kyra {

struct GameFlags {
	Common::Language lang;
	Common::Platform platform;

	bool isDemo					: 1;
	bool useAltShapeHeader		: 1;	// alternative shape header (uses 2 bytes more, those are unused though)
	bool isTalkie				: 1;
	bool useHiResOverlay		: 1;
	bool useDigSound			: 1;
	bool useInstallerPackage	: 1;

	byte gameID;
};

enum {
	GI_KYRA1 = 0,
	GI_KYRA2 = 1,
	GI_KYRA3 = 2
};

struct AudioDataStruct {
	const char * const *_fileList;
	const int _fileListLen;
	const void * const _cdaTracks;
	const int _cdaNumTracks;
};

// TODO: this is just the start of makeing the debug output of the kyra engine a bit more useable
// in the future we maybe merge some flags  and/or create new ones
enum kDebugLevels {
	kDebugLevelScriptFuncs = 1 << 0,		// prints debug output of o#_* functions
	kDebugLevelScript = 1 << 1,				// prints debug output of "EMCInterpreter" functions
	kDebugLevelSprites = 1 << 2,			// prints debug output of "Sprites" functions
	kDebugLevelScreen = 1 << 3,				// prints debug output of "Screen" functions
	kDebugLevelSound = 1 << 4,				// prints debug output of "Sound" functions
	kDebugLevelAnimator = 1 << 5,			// prints debug output of "ScreenAnimator" functions
	kDebugLevelMain = 1 << 6,				// prints debug output of common "KyraEngine(_v#)" functions && "TextDisplayer" functions
	kDebugLevelGUI = 1 << 7,				// prints debug output of "KyraEngine*" gui functions
	kDebugLevelSequence = 1 << 8,			// prints debug output of "SeqPlayer" functions
	kDebugLevelMovie = 1 << 9,				// prints debug output of movie specific funtions
	kDebugLevelTimer = 1 << 10				// prints debug output of "TimerManager" functions
};

enum kMusicDataID {
	kMusicIntro = 0,
	kMusicIngame,
	kMusicFinale
};

class Screen;
class Resource;
class Sound;
class Movie;
class TextDisplayer;
class StaticResource;
class TimerManager;
class Debugger;

class KyraEngine_v1 : public Engine {
friend class Debugger;
friend class ::KyraMetaEngine;
friend class GUI;
public:
	KyraEngine_v1(OSystem *system, const GameFlags &flags);
	virtual ~KyraEngine_v1();

	::GUI::Debugger *getDebugger();

	virtual void pauseEngineIntern(bool pause);

	bool quit() const { return _quitFlag; }

	uint8 game() const { return _flags.gameID; }
	const GameFlags &gameFlags() const { return _flags; }

	// access to Kyra specific functionallity
	Resource *resource() { return _res; }
	virtual Screen *screen() = 0;
	virtual TextDisplayer *text() { return _text; }
	Sound *sound() { return _sound; }
	StaticResource *staticres() { return _staticres; }
	TimerManager *timer() { return _timer; }

	uint32 tickLength() const { return _tickLength; }

	Common::RandomSource _rnd;

	// input
	void setMousePos(int x, int y);
	Common::Point getMousePos() const;

	// config specific
	bool speechEnabled();
	bool textEnabled();

	enum kVolumeEntry {
		kVolumeMusic = 0,
		kVolumeSfx = 1,
		kVolumeSpeech = 2
	};

	// volume reaches from 2 to 97
	void setVolume(kVolumeEntry vol, uint8 value);
	uint8 getVolume(kVolumeEntry vol);

	// quit handling
	virtual void quitGame();

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
	virtual int go() = 0;
	virtual int init();

	// quit Handling
	bool _quitFlag;

	// intern
	Resource *_res;
	Sound *_sound;
	TextDisplayer *_text;
	StaticResource *_staticres;
	TimerManager *_timer;
	EMCInterpreter *_emc;
	Debugger *_debugger;

	// config specific
	virtual void registerDefaultSettings();
	virtual void readSettings();
	virtual void writeSettings();

	uint8 _configWalkspeed;

	int _configMusic;
	bool _configSounds;
	uint8 _configVoice;

	// game speed
	virtual bool skipFlag() const = 0;
	virtual void resetSkipFlag(bool removeEvent = true) = 0;

	uint16 _tickLength;
	uint16 _gameSpeed;

	// run
	int8 _deathHandler;

	// timer
	virtual void setupTimers() = 0;

	virtual void setWalkspeed(uint8 speed) = 0;

	// detection
	GameFlags _flags;
	int _lang;

	// opcode
	virtual void setupOpcodeTable() = 0;
	Common::Array<const Opcode*> _opcodes;

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

	// items
	int _mouseState;

	virtual void setHandItem(uint16 item) = 0;
	virtual void removeHandItem() = 0;

	// game flags
	uint8 _flagsTable[100];	// TODO: check this value

	// sound
	Common::String _speechFile;

	int _curMusicTheme;
	int _curSfxFile;
	int16 _lastMusicCommand;

	const int8 *_trackMap;
	int _trackMapSize;

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

	const char *getSavegameFilename(int num);
	bool saveFileLoadable(int slot);

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;

		bool originalSave;	// savegame from original interpreter
		bool oldHeader;		// old scummvm save header
	};

	enum kReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	static kReadSaveHeaderError readSaveHeader(Common::InSaveFile *file, SaveHeader &header);

	Common::InSaveFile *openSaveForReading(const char *filename, SaveHeader &header);
	Common::OutSaveFile *openSaveForWriting(const char *filename, const char *saveName) const;
};

} // End of namespace Kyra

#endif

