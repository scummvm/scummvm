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

class Cursor;
class Encounter;
class MainMenu;
class ResourceManager;
class Scene;
class Screen;
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
	Scene           *scene()     { return _scene; }
	Screen          *screen()    { return _screen; }
	Sound           *sound()     { return _sound; }
	Text            *text()      { return _text; }
	Video           *video()     { return _video; }

	// Flags
	void setGameFlag(GameFlag flag);
	void clearGameFlag(GameFlag flag);
	void toggleGameFlag(GameFlag flag);
	bool isGameFlagSet(GameFlag flag) const;
	bool isGameFlagNotSet(GameFlag flag) const;

	// Misc
	uint getRandom(uint max) { return _rnd.getRandomNumber(max); }
	uint getRandomBit()      { return _rnd.getRandomBit(); }

	// Message handler
	MessageHandler *_handler;
	void switchMessageHandler(MessageHandler *handler);
	MessageHandler *getMessageHandler(uint32 index);

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
	Scene           *_scene;
	Screen          *_screen;
	Sound           *_sound;
	Text            *_text;
	Video           *_video;

	// Shared game data
	SharedData _data;

	bool _introPlaying;
	int  _gameFlags[1512];

	void handleEvents(bool doUpdate);
	void waitForTimer(uint32 msec_delay);
	void updateMouseCursor();
	void processDelayedEvents();

	/**
	 * Play the intro
	 */
	void playIntro();

	friend class Console;
};

} // namespace Asylum

#endif
