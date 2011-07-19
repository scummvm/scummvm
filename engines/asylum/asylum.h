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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ASYLUM_ENGINE_H
#define ASYLUM_ENGINE_H

#include "asylum/puzzles/data.h"
#include "asylum/resources/data.h"

#include "asylum/console.h"
#include "asylum/eventhandler.h"
#include "asylum/shared.h"

#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

/**
 * This is the namespace of the Asylum engine.
 *
 * Status of this engine:
 *  - Script interpreters for main game and encounters are implemented
 *  - Object handling, player reaction and special chapter logic is implemented
 *  - Scene parsing and drawing, movie playing, mouse cursor and menu handling are almost complete
 *  - Sound code is almost complete (music is still WIP)
 *  - Almost all puzzles are implemented
 *  - Walking is partialy implemented but the primitive pathfinding is missing
 *
 * Maintainers:
 *  alexbevi, alexandrefontoura, bluegr, littleboy, midstream
 *
 * Supported games:
 *  - Sanitarium
 */
namespace Asylum {

class Puzzle;
class Cursor;
class Encounter;
class Menu;
class Reaction;
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
	virtual GUI::Debugger *getDebugger() { return _console; }

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
	void startGame(ResourcePackId sceneId, StartGameType type);

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
	void switchScene(ResourcePackId sceneId) { startGame(sceneId, kStartGameScene); }

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
	Reaction        *reaction()  { return _reaction; }
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

	SharedData      *data()       { return &_data; }
	PuzzleData      *puzzleData() { return &_puzzleData; }

	// Flags
	void setGameFlagByIndex(int32 index);
	int32 getGameFlagByIndex(int32 index);
	void setGameFlag(GameFlag flag);
	void clearGameFlag(GameFlag flag);
	void toggleGameFlag(GameFlag flag);
	bool isGameFlagSet(GameFlag flag) const;
	bool isGameFlagNotSet(GameFlag flag) const;
	void resetFlags();

	// Misc
	uint getRandom(uint max) { return _rnd->getRandomNumber(max); }
	uint getRandomBit()      { return _rnd->getRandomBit(); }

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
	 * Gets a message handler.
	 *
	 * @param index Zero-based index of the message handler
	 *
	 * @return The message handler.
	 */
	EventHandler* getPuzzle(uint32 index);

	/**
	 * Updates the reverse stereo scene status from the config
	 */
	void updateReverseStereo();

	/**
	 * Gets a sine cosine pair.
	 *
	 * @param index1 The first index.
	 * @param index2 The second index.
	 *
	 * @return The sine cosine values.
	 */
	Common::Point getSinCosValues(int32 index1, int32 index2);

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s);

private:
	const ADGameDescription *_gameDescription;

	// Misc
	Console              *_console;
	Common::RandomSource *_rnd;

	// Game
	Cursor          *_cursor;
	Encounter       *_encounter;
	Menu            *_menu;
	Reaction        *_reaction;
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
	Puzzle *_puzzles[17];

	// Game data
	PuzzleData _puzzleData;
	SharedData _data;
	int  _gameFlags[190];
	int32 _sinCosTables[72];
	bool _introPlayed;
	int32 _tickOffset;

	void updateMouseCursor();
	void processDelayedEvents();

	/**
	 * Play the intro
	 */
	void playIntro();

	/**
	 * Initializes the puzzles
	 */
	void initPuzzles();

	/**
	 * Initializes the sine/cosine tables.
	 */
	void initSinCosTables(double a2, int32 a3, int32 a4);

	/**
	 * Calculate the offset into the sine/cosine tables.
	 *
	 * @param val The value.
	 *
	 * @return The calculated offset.
	 */
	int32 computeSinCosOffset(int32 val);


	// Debug
	friend class Console;
	Scene *_previousScene;
	ResourcePackId _delayedSceneIndex;
	int32 _delayedVideoIndex;
};

} // namespace Asylum

#endif
