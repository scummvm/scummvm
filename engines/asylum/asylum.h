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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_ASYLUM_H
#define ASYLUM_ASYLUM_H

#include "common/file.h"
#include "common/language.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "asylum/resources/data.h"

#include "asylum/console.h"
#include "asylum/eventhandler.h"
#include "asylum/shared.h"

/**
 * This is the namespace of the Asylum engine.
 *
 * Status of this engine: Complete
 *
 * Games using this engine:
 * - Sanitarium
 */

struct ADGameDescription;

namespace Asylum {

class Cursor;
class Encounter;
class Menu;
class Puzzles;
class ResourceManager;
class Savegame;
class Scene;
class Screen;
class ScriptManager;
class Special;
class Speech;
class Sound;
class Text;
class VideoPlayer;

class AsylumEngine: public Engine, public Common::Serializable {
protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;

public:
	enum StartGameType {
		kStartGamePlayIntro,
		kStartGameLoad,
		kStartGameScene
	};

	AsylumEngine(OSystem *system, const ADGameDescription *gd);
	virtual ~AsylumEngine();

	/**
	 * Start a new the game
	 */
	bool startGame(ResourcePackId sceneId, StartGameType type);

	/**
	 * Restarts the game
	 */
	void restart();

	/**
	 * Run event loop
	 */
	void handleEvents();

	/**
	 * Switch to a new scene
	 *
	 * @param sceneId ResourcePack for the scene
	 */
	void switchScene(ResourcePackId sceneId) { (void)startGame(sceneId, kStartGameScene); }

	/**
	 * Get the number of engine ticks
	 *
	 * @return The tick.
	 */
	uint32 getTick() { return _system->getMillis() + _tickOffset; }

	/**
	 * Sets the tick value
	 *
	 * @param offset The offset.
	 */
	void setTick(uint32 offset) { _tickOffset = offset - _system->getMillis(); }

	/**
	 * Resets the game
	 */
	void reset();

	/**
	 * This is the global tick counter.
	 */
	uint32 screenUpdateCount;
	uint32 lastScreenUpdate;

	// Game
	Cursor          *cursor()    { return _cursor; }
	Encounter       *encounter() { return _encounter; }
	Menu            *menu()      { return _menu; }
	ResourceManager *resource()  { return _resource; }
	Savegame        *savegame()  { return _savegame; }
	Scene           *scene()     { return _scene; }
	Screen          *screen()    { return _screen; }
	ScriptManager   *script()    { return _script; }
	Special         *special()   { return _special; }
	Speech          *speech()    { return _speech; }
	Sound           *sound()     { return _sound; }
	Text            *text()      { return _text; }
	VideoPlayer     *video()     { return _video; }

	SharedData      *data()      { return &_data; }
	Puzzles         *puzzles()   { return _puzzles; }

	// Flags
	void setGameFlag(GameFlag flag);
	void clearGameFlag(GameFlag flag);
	void toggleGameFlag(GameFlag flag);
	bool isGameFlagSet(GameFlag flag) const;
	bool isGameFlagNotSet(GameFlag flag) const;
	bool areGameFlagsSet(uint from, uint to) const;
	void resetFlags() { memset(_gameFlags, 0, sizeof(_gameFlags)); }

	// Misc
	uint getRandom(uint max) { return max ? _rnd->getRandomNumber(max - 1) : 0; }
	uint getRandomBit()      { return _rnd->getRandomBit(); }

	bool rectContains(const int16 (*rectPtr)[4], const Common::Point &p) const;

	// Steam achievements
	void unlockAchievement(const Common::String &id);
	void checkAchievements();

	/**
	 * Switch message handler.
	 *
	 * @param handler If non-null, a pointer to an EventHandler class.
	 */
	void switchEventHandler(EventHandler *handler);

	/**
	 * Notifies the current event handler of an event
	 *
	 * @param type The event type.
	 */
	void notify(AsylumEventType type, int32 param1 = 0, int32 param2 = 0);

	/**
	 * Updates the reverse stereo scene status from the config
	 */
	void updateReverseStereo();

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s);

	bool checkGameVersion(const char *version) { return !strcmp(_gameDescription->extra, version); }
	bool isAltDemo() { return Common::File::exists("asylum.dat"); }
	Common::Language getLanguage() { return _gameDescription->language; }
	Common::String getTargetName() { return _targetName; }
	Common::String getMoviesFileName() { return Common::String::format("%s.movies", _targetName.c_str()); }
	bool isMenuVisible() { return _handler == (EventHandler *)_menu; }
	EventHandler *getEventHandler() { return _handler; }

	// Save/Load
	int getAutosaveSlot() const { return getMetaEngine()->getAutosaveSlot(); }
	bool canLoadGameStateCurrently();
	Common::Error loadGameState(int slot);
	bool canSaveGameStateCurrently();
	bool canSaveAutosaveCurrently();
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false);

private:
	const ADGameDescription *_gameDescription;

	// Misc
	Console              *_console;
	Common::RandomSource *_rnd;

	// Game
	Cursor          *_cursor;
	Encounter       *_encounter;
	Menu            *_menu;
	ResourceManager *_resource;
	Savegame        *_savegame;
	Scene           *_scene;
	Screen          *_screen;
	ScriptManager   *_script;
	Special         *_special;
	Speech          *_speech;
	Sound           *_sound;
	Text            *_text;
	VideoPlayer     *_video;

	// Current EventHandler class instance
	EventHandler *_handler;

	// Game data
	Puzzles     *_puzzles;
	SharedData   _data;
	uint32       _gameFlags[130];
	bool         _introPlayed;
	int32        _tickOffset;

	void updateMouseCursor();
	void processDelayedEvents();

	/**
	 * Play the intro
	 */
	void playIntro();

	// Debug
	friend class Console;
	Scene *_previousScene;
	ResourcePackId _delayedSceneIndex;
	int32 _delayedVideoIndex;
};

} // namespace Asylum

#endif // ASYLUM_ASYLUM_H
