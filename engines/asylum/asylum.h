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

enum FlagType {
	kFlagType1 = 0,
	kFlagType2
};

// XXX
// If defined, this will play the scene title loading up
// progress before the scene is entered. This is
// just a convenience, as there's no need for the type
// of pre-loading that was performed in the original
// #define SHOW_SCENE_LOADING

// XXX If defined, this flag will prevent the intro movies
// from being played whenever the engine is started
#define SKIP_INTRO

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
	 *
	 * TODO rename?
	 */
	uint32 tempTick07;

	// Game
	Video* video()   { return _video;	}
	Sound* sound()   { return _sound; }
	Screen* screen() { return _screen; }
	Scene* scene()   { return _scene;}
	Text* text()     { return _text; }

	// Flags
	void setGameFlag(int flag);
	void clearGameFlag(int flag);
	void toggleGameFlag(int flag);
	bool isGameFlagSet(int flag);
	bool isGameFlagNotSet(int flag);

	// Misc
	uint getRandom(uint max) { return _rnd.getRandomNumber(max); }
	uint getRandomBit()      { return _rnd.getRandomBit(); }

	// Flags
	void setFlag(FlagType flag)   { setFlag(flag, true); }
	void clearFlag(FlagType flag) { setFlag(flag, false); }

private:
	const ADGameDescription *_gameDescription;

	// Misc
	Console              *_console;
	Common::RandomSource  _rnd;

	// Game
	//Encounter *_encounter;
	MainMenu  *_mainMenu;
	Scene     *_scene;
	Screen    *_screen;
	Sound     *_sound;
	Text      *_text;
	Video     *_video;

	bool _introPlaying;
	int _gameFlags[1512];
	bool _flags[2];

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
