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

#include "common/random.h"

#include "engines/engine.h"

#include "asylum/staticres.h"
#include "asylum/console.h"
#include "asylum/views/scene.h"
#include "asylum/views/menu.h"
#include "asylum/system/screen.h"
#include "asylum/system/sound.h"
#include "asylum/system/video.h"
//#include "asylum/blowuppuzzle.h"
//#include "asylum/encounters.h"
#include "asylum/system/text.h"

namespace Asylum {

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

class Console;
class Scene;
class MainMenu;
class Scene;
class Screen;
class Sound;
class Video;
class Encounter;

enum kDebugLevels {
	kDebugLevelMain      = 1 << 0,
	kDebugLevelResources = 1 << 1,
	kDebugLevelSprites   = 1 << 2,
	kDebugLevelInput     = 1 << 3,
	kDebugLevelMenu      = 1 << 4,
	kDebugLevelScripts   = 1 << 5,
	kDebugLevelSound     = 1 << 6,
	kDebugLevelSavegame  = 1 << 7,
	kDebugLevelScene     = 1 << 8,
	kDebugLevelBarriers  = 1 << 9
};

class AsylumEngine: public Engine {
public:

	AsylumEngine(OSystem *system, Common::Language language);
	virtual ~AsylumEngine();

	/** .text:0040F430
	 * Initalize the game environment
	 */
	Common::Error init();

	/** .text:0041A500
	 * Start the game environment
	 */
	Common::Error go();
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;

	/** .text:00415830
	 * Start a new the game
	 */
	void startGame();

	void setGameFlag(int flag);
	void clearGameFlag(int flag);
	void toggleGameFlag(int flag);
	bool isGameFlagSet(int flag);
	bool isGameFlagNotSet(int flag);

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

	Video* video() { return _video;	}
	Sound* sound() { return _sound; }
	Screen* screen() { return _screen; }
	Scene* scene() { return _scene;}
	Text* text() { return _text; }

	Common::RandomSource _rnd;

private:
	void checkForEvent(bool doUpdate);
	void waitForTimer(int msec_delay);
	void updateMouseCursor();
	void processDelayedEvents();

	/** .text:0041B630
	 * Start a new the game
	 */
	void playIntro();

	Common::Language _language;

	bool _introPlaying;

	Console   *_console;
	Scene     *_scene;
	MainMenu  *_mainMenu;
	Screen    *_screen;
	Sound     *_sound;
	Video     *_video;
	Text      *_text;
	//Encounter *_encounter;

	int _gameFlags[1512];

	friend class Console;
};

} // namespace Asylum

#endif
