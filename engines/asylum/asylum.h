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

#include "engines/engine.h"

#include "asylum/console.h"
#include "asylum/scene.h"
#include "asylum/menu.h"
#include "asylum/screen.h"
#include "asylum/sound.h"
#include "asylum/video.h"
#include "asylum/blowuppuzzle.h"
#include "asylum/encounters.h"

namespace Asylum {

// XXX
// If defined, this will play the scene title loading
// progress before the scene is entered. This is
// just a convenience, as there's no need for the type
// of pre-loading that was performed in the original
#define SHOW_SCENE_LOADING

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
	kDebugLevelScene     = 1 << 8
};

class AsylumEngine: public Engine {
public:

	AsylumEngine(OSystem *system, Common::Language language);
	virtual ~AsylumEngine();

	// Engine APIs
	Common::Error init();
	Common::Error go();
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;

	void setGameFlag(int flag);
	void clearGameFlag(int flag);
	void toggleGameFlag(int flag);
	bool isGameFlagSet(int flag);
	bool isGameFlagNotSet(int flag);

	Video*  video()  {
		return _video;
	}
	Sound*  sound()  {
		return _sound;
	}
	Screen* screen() {
		return _screen;
	}
	Scene*  scene()  {
		return _scene;
	}
	Text* text() {
		return _text;
	}

	int ambientVolume() {
		return _ambientVolume;
	}
	int soundVolume()   {
		return _soundVolume;
	}

private:
	void checkForEvent(bool doUpdate);
	void waitForTimer(int msec_delay);
	void updateMouseCursor();
	void processDelayedEvents();
	void playIntro();

	Common::Language     _language;
	Common::RandomSource _rnd;

	int _ambientVolume;
	int _soundVolume;

	bool _introPlaying;

	Console   *_console;
	Scene     *_scene;
	MainMenu  *_mainMenu;
	Screen    *_screen;
	Sound     *_sound;
	Video     *_video;
	Text      *_text;
	Encounter *_encounter;

	int _gameFlags[1512];

	friend class Console;
};

} // namespace Asylum

#endif
