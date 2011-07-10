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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/fs.h"
#include "engines/util.h"
#include "cge/cge.h"
#include "cge/vga13h.h"
#include "cge/cge_main.h"
#include "cge/talk.h"
#include "cge/text.h"
#include "cge/vol.h"


namespace CGE {

CGEEngine::CGEEngine(OSystem *syst, const ADGameDescription *gameDescription)
	: Engine(syst), _gameDescription(gameDescription), _randomSource("cge") {

	// Debug/console setup
	DebugMan.addDebugChannel(kCGEDebug, "general", "CGE general debug channel");

	_isDemo      = _gameDescription->flags & ADGF_DEMO;
	_startupMode = 1;
	_demoText    = DEMO_TEXT;
	_oldLev      = 0;
	_jbw         = false;
	_pocPtr      = 0;

}

void CGEEngine::setup() {
	// Initialise fields
	_lastFrame = 0;
	_hero = NULL;

	// Create debugger console
	_console = new CGEConsole(this);

	// Initialise classes that have static members
	Vga::init();
	VFile::init();
	Bitmap::init();
	Talk::init();

	// Initialise sprite arrays used by game objects
	LI[0] = new Bitmap("LITE0", true);
	LI[1] = new Bitmap("LITE1", true);
	LI[2] = new Bitmap("LITE2", true);
	LI[3] = new Bitmap("LITE3", true);
	LI[4] = NULL;

	// Initialise engine objects
	_text = new Text(this, progName(), 128);
	_vga = new Vga(M13H);
	_heart = new Heart;
	_sys = new System(this);
	_pocLight = new Sprite(this, LI);
	for (int i = 0; i < POCKET_NX; i++)
		_pocket[i] = new Sprite(this, NULL);
	_sprite = new Sprite(this, NULL);
	_miniCave = new Sprite(this, NULL);
	_shadow = new Sprite(this, NULL);
	_horzLine = new HorizLine(this);
	_infoLine = new InfoLine(this, INFO_W);
	_cavLight = new CavLight(this);
	_debugLine = new InfoLine(this, SCR_WID);
	_snail = new Snail(this, false);
	_snail_ = new Snail(this, true);

	_mouse = new Mouse(this);
	_keyboard = new Keyboard();
	_eventManager = new EventManager();
	_offUseCount = atoi(_text->getText(OFF_USE_COUNT));
	_music = true;

	for (int i = 0; i < POCKET_NX; i++)
		_pocref[i] = -1;
	_volume[0] = 0;
	_volume[1] = 0;

	if (_isDemo) {
		_maxCaveArr[0] = CAVE_MAX;
		_maxCaveArr[1] = -1;
		_maxCaveArr[2] = -1;
		_maxCaveArr[3] = -1;
		_maxCaveArr[4] = -1;
	} else {
		_maxCaveArr[0] = 1;
		_maxCaveArr[1] = 8;
		_maxCaveArr[2] = 16;
		_maxCaveArr[3] = 23;
		_maxCaveArr[4] = 24;
	};
	_maxCave  =  0;
	_dark     = false;
	_game     = false;
	_now      =  1;
	_lev      = -1;

	for (int i = 0; i < 4; i++)
		_flag[i] = false;

}

CGEEngine::~CGEEngine() {
	debug("CGEEngine::~CGEEngine");

	// Call classes with static members to clear them up
	Talk::deinit();
	Bitmap::deinit();
	VFile::deinit();
	Vga::deinit();

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

	delete _console;

	// Delete engine objects 
	delete _sprite;
	delete _miniCave;
	delete _shadow;
	delete _horzLine;
	delete _infoLine;
	delete _cavLight;
	delete _debugLine;
	delete LI[0];
	delete LI[1];
	delete LI[2];
	delete LI[3];
	delete _text;
	delete _heart;
	delete _pocLight;
	delete _keyboard;
	delete _mouse;
	for (int i = 0; i < POCKET_NX; i++)
		delete _pocket[i];
	delete _snail;
	delete _snail_;
	delete _hero;
	delete _vga;
	delete _sys;
}

Common::Error CGEEngine::run() {
	// Initialize graphics using following:
	initGraphics(320, 200, false);

	// Setup necessary game objects
	setup();

	// Additional setup.
	debug("CGEEngine::init");

	cge_main();

	return Common::kNoError;
}

} // End of namespace CGE
