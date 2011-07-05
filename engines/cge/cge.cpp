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
#include "cge/bitmaps.h"
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
	_horzLine = new Sprite(this, HL);
	_infoLine = new InfoLine(this, INFO_W);
	_cavLight = new Sprite(this, PR);
	_debugLine = new InfoLine(this, SCR_WID);
	MB[0] = new Bitmap("BRICK", true);
	MB[1] = NULL;
	HL[0] = new Bitmap("HLINE", true);
	HL[1] = NULL;
	MC[0] = new Bitmap("MOUSE", true);
	MC[1] = new Bitmap("DUMMY", true);
	MC[2] = NULL;
	PR[0] = new Bitmap("PRESS", true);
	PR[1] = NULL;
	SP[0] = new Bitmap("SPK_L", true);
	SP[1] = new Bitmap("SPK_R", true);
	SP[2] = NULL;
	LI[0] = new Bitmap("LITE0", true);
	LI[1] = new Bitmap("LITE1", true);
	LI[2] = new Bitmap("LITE2", true);
	LI[3] = new Bitmap("LITE3", true);
	LI[4] = NULL;
	_snail = new Snail(this, false);
	_snail_ = new Snail(this, true);

	_mouse = new MOUSE(this);
	_keyboard = new Keyboard();
	_eventManager = new EventManager();
	_offUseCount = atoi(_text->getText(OFF_USE_COUNT));
	_music = true;

	for (int i = 0; i < POCKET_NX; i++)
		_pocref[i] = -1;
	_volume[0] = 0;
	_volume[1] = 0;

	_savTab[0].Ptr = &_now;
	_savTab[0].Len = sizeof(_now);
	_savTab[0].Flg = 1;
	_savTab[1].Ptr = &_oldLev;
	_savTab[1].Len = sizeof(_oldLev);
	_savTab[1].Flg = 1;
	_savTab[2].Ptr = &_demoText;
	_savTab[2].Len = sizeof(_demoText);
	_savTab[2].Flg = 1;
	_savTab[3].Ptr = &_game;
	_savTab[3].Len = sizeof(_game);
	_savTab[3].Flg = 1;
	_savTab[4].Ptr = &_game;
	_savTab[4].Len = sizeof(_game);
	_savTab[4].Flg = 1;
	_savTab[5].Ptr = &_game;
	_savTab[5].Len = sizeof(_game);
	_savTab[5].Flg = 1;
	_savTab[6].Ptr = &_game;
	_savTab[6].Len = sizeof(_game);
	_savTab[6].Flg = 1;
	_savTab[7].Ptr = &_game;
	_savTab[7].Len = sizeof(_game);
	_savTab[7].Flg = 1;
	_savTab[8].Ptr = &_vga->_mono;
	_savTab[8].Len = sizeof(_vga->_mono);
	_savTab[8].Flg = 0;
	_savTab[9].Ptr = &_music;
	_savTab[9].Len = sizeof(_music);
	_savTab[9].Flg = 1;
	_savTab[10].Ptr = _volume;
	_savTab[10].Len = sizeof(_volume);
	_savTab[10].Flg = 1;
	_savTab[11].Ptr = _flag;
	_savTab[11].Len = sizeof(_flag);
	_savTab[11].Flg = 1;
	_savTab[12].Ptr = _heroXY;
//	_savTab[12].Len = sizeof(_heroXY); FIXME: illegal sizeof
	_savTab[12].Len = 0;
	_savTab[12].Flg = 1;
	_savTab[13].Ptr = _barriers;
//	_savTab[13].Len = sizeof(_barriers); FIXME: illegal sizeof
	_savTab[13].Len = 0;
	_savTab[13].Flg = 1;
	_savTab[14].Ptr = _pocref;
	_savTab[14].Len = sizeof(_pocref);
	_savTab[14].Flg = 1;
	_savTab[15].Ptr = NULL;
	_savTab[15].Len = 0;
	_savTab[15].Flg = 0;
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
	delete MB[0];
	delete HL[0];
	delete MC[0];
	delete MC[1];
	delete PR[0];
	delete SP[0];
	delete SP[1];
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
