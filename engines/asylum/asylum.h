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

#include "asylum/console.h"
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

// XXX
// I'm not sure if system endian-ness would have any
// effect on the byte order of the data files, but I guess
// it won't hurt to keep this here until we can test
// on a big-endian system
#ifndef SCUMM_BIG_ENDIAN
#define LOBYTE(word) (word & 0xFF)
#else
#define LOBYTE(word) ((word >> 24) & 0xFF)
#endif

//////////////////////////////////////////////////////////////////////////
// Flags
enum FlagType {
	kFlagType1 = 0,
	kFlagType2,
	kFlagTypeSkipDraw,
	kFlagTypeSceneRectChanged
};

class Encounter;
class MainMenu;
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
	typedef Common::Functor1<Common::Event &, void> MessageHandler;

	AsylumEngine(OSystem *system, const ADGameDescription *gd);
	virtual ~AsylumEngine();

	/**
	 * Start a new the game
	 */
	void startGame();

	/**
	 * Wrapper function to the OSystem getMillis() method
	 */
	int32 getTick() { return (int32)_system->getMillis(); }

	/**
	 * This is the global tick counter.
	 */
	uint32 globalTickValue;
	uint32 globalTickValue_2;

	// Game
	Encounter *encounter() { return _encounter; }
	MainMenu  *menu()      { return _mainMenu; }
	Scene     *scene()     { return _scene; }
	Screen    *screen()    { return _screen; }
	Sound     *sound()     { return _sound; }
	Text      *text()      { return _text; }
	Video     *video()     { return _video; }

	// Flags
	void setGameFlag(GameFlag flag);
	void clearGameFlag(GameFlag flag);
	void toggleGameFlag(GameFlag flag);
	bool isGameFlagSet(GameFlag flag);
	bool isGameFlagNotSet(GameFlag flag);

	// Misc
	uint getRandom(uint max) { return _rnd.getRandomNumber(max); }
	uint getRandomBit()      { return _rnd.getRandomBit(); }

	// Flags
	void setFlag(FlagType flag)   { setFlag(flag, true); }
	void clearFlag(FlagType flag) { setFlag(flag, false); }

	// Message handler
	void switchMessageHandler(MessageHandler *handler);
	MessageHandler *getMessageHandler(uint32 index);

private:
	const ADGameDescription *_gameDescription;

	// Misc
	Console              *_console;
	Common::RandomSource  _rnd;

	// Game
	Encounter *_encounter;
	MainMenu  *_mainMenu;
	Scene     *_scene;
	Screen    *_screen;
	Sound     *_sound;
	Text      *_text;
	Video     *_video;

	bool _introPlaying;
	int _gameFlags[1512];
	bool _flags[4];

	void handleEvents(bool doUpdate);
	void waitForTimer(int msec_delay);
	void updateMouseCursor();
	void processDelayedEvents();

	/**
	 * Play the intro
	 */
	void playIntro();

	void setFlag(FlagType flag, bool isSet);

	friend class Console;
};

} // namespace Asylum

#endif
