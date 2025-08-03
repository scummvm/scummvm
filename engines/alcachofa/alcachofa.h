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

#ifndef ALCACHOFA_H
#define ALCACHOFA_H

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

#include "alcachofa/detection.h"
#include "alcachofa/camera.h"
#include "alcachofa/input.h"
#include "alcachofa/sounds.h"
#include "alcachofa/player.h"
#include "alcachofa/scheduler.h"
#include "alcachofa/console.h"
#include "alcachofa/game.h"

namespace Alcachofa {

class IDebugHandler;
class IRenderer;
class DrawQueue;
class World;
class Script;
class GlobalUI;
class Menu;
class Game;
struct AlcachofaGameDescription;

enum class SaveVersion : Common::Serializer::Version {
	Initial = 0
};

class Config {
public:
	Config();

	inline bool &subtitles() { return _subtitles; }
	inline bool &highQuality() { return _highQuality; }
	inline bool &bits32() { return _bits32; }
	inline uint8 &musicVolume() { return _musicVolume; }
	inline uint8 &speechVolume() { return _speechVolume; }

	void loadFromScummVM();
	void saveToScummVM();

private:
	bool
		_subtitles = true,
		_highQuality = true,
		_bits32 = true;
	uint8
		_musicVolume = 255,
		_speechVolume = 255;
};

class AlcachofaEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
protected:
	// Engine APIs
	Common::Error run() override;
public:
	AlcachofaEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~AlcachofaEngine() override;

	inline IRenderer &renderer() { return *_renderer; }
	inline DrawQueue &drawQueue() { return *_drawQueue; }
	inline Camera &camera() { return _camera; }
	inline Input &input() { return _input; }
	inline Sounds &sounds() { return _sounds; }
	inline Player &player() { return *_player; }
	inline World &world() { return *_world; }
	inline Script &script() { return *_script; }
	inline GlobalUI &globalUI() { return *_globalUI; }
	inline Menu &menu() { return *_menu; }
	inline Scheduler &scheduler() { return _scheduler; }
	inline Console &console() { return *_console; }
	inline Game &game() { return *_game; }
	inline Config &config() { return _config; }
	inline bool isDebugModeActive() const { return _debugHandler != nullptr; }

	uint32 getMillis() const;
	void setMillis(uint32 newMillis);
	virtual void pauseEngineIntern(bool pause);
	void playVideo(int32 videoId);
	void fadeExit();
	void setDebugMode(DebugMode debugMode, int32 param);

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
		// TODO: Implement
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		// TODO: Implement
		return true;
	}

	Common::Error syncGame(Common::Serializer &s);
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

private:
	bool tryLoadFromLauncher();

	Console *_console = new Console();
	Common::ScopedPtr<IDebugHandler> _debugHandler;
	Common::ScopedPtr<IRenderer> _renderer;
	Common::ScopedPtr<DrawQueue> _drawQueue;
	Common::ScopedPtr<World> _world;
	Common::ScopedPtr<Script> _script;
	Common::ScopedPtr<Player> _player;
	Common::ScopedPtr<GlobalUI> _globalUI;
	Common::ScopedPtr<Menu> _menu;
	Common::ScopedPtr<Game> _game;
	Camera _camera;
	Input _input;
	Sounds _sounds;
	Scheduler _scheduler;
	Config _config;

	uint32 _timeNegOffset = 0, _timePosOffset = 0;
	uint32 _timeBeforePause = 0;
};

extern AlcachofaEngine *g_engine;
#define SHOULD_QUIT ::Alcachofa::g_engine->shouldQuit()

} // End of namespace Alcachofa

#endif // ALCACHOFA_H
