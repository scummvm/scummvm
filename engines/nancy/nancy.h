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
 */

#ifndef NANCY_H
#define NANCY_H

#include "nancy/console.h"
#include "nancy/detection.h"
#include "nancy/time.h"

#include "engines/engine.h"
#include "common/file.h"
#include "common/str.h"

namespace Common {
class RandomSource;
class Serializer;
}

/**
 * This is the namespace of the Nancy engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Nancy Drew 1
 * - ...
 */
namespace Nancy {

static const int kSavegameVersion = 1;

enum NancyDebugChannels {
	kDebugEngine    	= 1 << 0,
	kDebugActionRecord  = 1 << 1,
	kDebugScene			= 1 << 2
};

struct NancyGameDescription;

class ResourceManager;
class IFF;
class InputManager;
class SoundManager;
class GraphicsManager;
class CursorManager;
class CheatDialog;

namespace State {
class State;
class Logo;
class Scene;
class Map;
class Help;
class Credits;
}

class NancyEngine : public Engine {
public:
	friend class NancyConsole;
	friend class State::Logo; // TODO

	enum GameState {
		kBoot,
		kPartnerLogo,
		kLogo,
		kCredits,
		kMap,
		kMainMenu,
		kLoadSave,
		kSetup,
		// unknown/invalid
		kHelp,
		kScene,
		// CD change
		kCheat,
		kQuit,
		// regain focus
		kNone,
		kPause, // only used when the GMM is on screen
		kReloadSave
	};

	NancyEngine(OSystem *syst, const NancyGameDescription *gd);
	~NancyEngine();

	GUI::Debugger *getDebugger();

	uint32 getFeatures() const;
	const char *getGameId() const;

	void initGame(const NancyGameDescription *gd);

	GameType getGameType() const;
	Common::Platform getPlatform() const;

	bool hasFeature(EngineFeature f) const;
	const char *getCopyrightString() const;

	void syncSoundSettings();

	static NancyEngine *create(GameType type, OSystem *syst, const NancyGameDescription *gd);

	// Chunks found in BOOT get extracted and cached at startup, this function lets other classes access them
	Common::SeekableReadStream *getBootChunkStream(const Common::String &name);

	// Used for state switching
	void stopAndUnloadSpecificSounds();
	
	void setState(GameState state, GameState overridePrevious = kNone);
	void setPreviousState();

	void callCheatMenu(bool eventFlags) { setState(kCheat), _cheatTypeIsEventFlag = eventFlags; }

	void setMouseEnabled(bool enabled);

	virtual Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	virtual Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	virtual bool canLoadGameStateCurrently() override { return canSaveGameStateCurrently(); };
	virtual bool canSaveGameStateCurrently() override;

	// Managers
	ResourceManager *resource;
	GraphicsManager *graphicsManager;
	CursorManager *cursorManager;
	InputManager *input;
	SoundManager *sound;
	
	OSystem *_system;
	Common::RandomSource *_rnd;
	const NancyGameDescription *_gameDescription;
	bool launchConsole;
	
	uint16 firstSceneID;
	uint16 startTimeHours;

	bool overrideMovementTimeDeltas;
	Time slowMovementTimeDelta;
	Time fastMovementTimeDelta;

protected:
	// Engine APIs
	Common::Error run();

	void bootGameEngine();

	State::State *getStateObject(GameState state);

	bool addBootChunk(const Common::String &name, Common::SeekableReadStream *stream);
	void clearBootChunks();

	Common::Error synchronize(Common::Serializer &serializer);

	enum {
		kMaxFilenameLen = 32
	};

	struct Image {
		Common::String name;
		uint16 width;
		uint16 height;
	};

	struct GameFlow {
		State::State *currentState = nullptr;
		State::State *previousState = nullptr;
	};

	bool _cheatTypeIsEventFlag;

	void preloadCals(const IFF &boot);
	void readChunkList(const IFF &boot, Common::Serializer &ser, const Common::String &prefix);
	Common::String readFilename(Common::ReadStream *stream) const;

	virtual uint getFilenameLen() const { return 9; };
	virtual void readBootSummary(const IFF &boot);

private:
	GameFlow _gameFlow;

	NancyConsole *_console;
	GameType _gameType;
	Common::Platform _platform;

	Common::HashMap<Common::String, Common::SeekableReadStream *> _bootChunks;
};

#define NanEngine (*((NancyEngine *)(g_engine)))

} // End of namespace Nancy

#endif // NANCY_H
