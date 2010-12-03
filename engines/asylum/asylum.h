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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_ENGINE_H
#define ASYLUM_ENGINE_H

#include "asylum/resources/data.h"

#include "asylum/console.h"
#include "asylum/eventhandler.h"
#include "asylum/shared.h"

#include "common/random.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

/**
 * This is the namespace of the Asylum engine.
 *
 * Status of this engine:
 *  - Code for scrolling and showing up objects and actors (properly clipped) is present
 *  - A preliminary script interpreter is implemented, and game scripts are read and partially parsed,
 *    actors are now drawn in the scene, and there is some interaction with the environment (e.g. "examine" actions).
 *  - Movie code is almost complete
 *  - Scene information is partially read, and the scene hotspots are created correctly.
 *  - Mouse cursor is initialized and animated properly
 *  - Game texts and game fonts are read correctly
 *  - Preliminary code for walking around with the mouse.
 *  - Some of the menu screens are working (like, for example, the credits screen)
 *
 * Maintainers:
 *  alexbevi, alexandrefontoura, bluegr
 *
 * Supported games:
 *  - Sanitarium
 */
namespace Asylum {

// If defined, will show the scene update times on the debugger output
//#define DEBUG_SCENE_TIMES

class BlowUpPuzzle;
class Cursor;
class Encounter;
class MainMenu;
class ResourceManager;
class Savegame;
class Scene;
class Screen;
class ScriptManager;
class Special;
class Speech;
class Sound;
class Text;
class Video;

class AsylumEngine: public Engine {
protected:
	// Engine APIs
	virtual Common::Error run();
	virtual void errorString(const char *buf_input, char *buf_output, int buf_output_size);
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
	 * Switch to a new scene
	 *
	 * @param sceneId ResourcePack for the scene
	 */
	void switchScene(ResourcePackId sceneId) { startGame(sceneId, kStartGameScene); }

 	/**
	 * Wrapper function to the OSystem getMillis() method
 	 */
	uint32 getTick() { return _system->getMillis(); }

	/**
	 * Gets the shared data.
	 *
	 * @return a pointer to the shared data.
	 */
	SharedData *getData() { return &_data; }

	/**
	 * This is the global tick counter.
	 */
	uint32 screenUpdateCount;
	uint32 lastScreenUpdate;

	// Game
	Cursor          *cursor()    { return _cursor; }
	Encounter       *encounter() { return _encounter; }
	MainMenu        *menu()      { return _mainMenu; }
	ResourceManager *resource()  { return _resource; }
	Savegame        *savegame()  { return _savegame; }
	Scene           *scene()     { return _scene; }
	Screen          *screen()    { return _screen; }
	ScriptManager   *script()    { return _script; }
	Special         *special()   { return _special; }
	Speech          *speech()    { return _speech; }
	Sound           *sound()     { return _sound; }
	Text            *text()      { return _text; }
	Video           *video()     { return _video; }

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
	uint getRandom(uint max) { return _rnd.getRandomNumber(max); }
	uint getRandomBit()      { return _rnd.getRandomBit(); }

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
	void notify(AsylumEventType type);

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

private:
	const ADGameDescription *_gameDescription;

	// Misc
	Console              *_console;
	Common::RandomSource  _rnd;

	// Game
	Cursor          *_cursor;
	Encounter       *_encounter;
	MainMenu        *_mainMenu;
	ResourceManager *_resource;
	Savegame        *_savegame;
	Scene           *_scene;
	Screen          *_screen;
	ScriptManager   *_script;
	Special         *_special;
	Speech          *_speech;
	Sound           *_sound;
	Text            *_text;
	Video           *_video;

	// Current EventHandler class instance
	EventHandler *_handler;
	BlowUpPuzzle *_puzzles[16];

	// Game data
	SharedData _data;
	int  _gameFlags[145];
	bool _introPlayed;
	int32 _sinCosTables[72];

	void handleEvents();
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

	/**
	 * Sets up the loaded game.
	 */
	void setupLoadedGame();

	friend class Console;
};

} // namespace Asylum

#endif
