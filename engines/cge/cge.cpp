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
	: Engine(syst), _gameDescription(gameDescription) {

	// Debug/console setup
	DebugMan.addDebugChannel(kCGEDebug, "general", "CGE general debug channel");

	_isDemo = _gameDescription->flags & ADGF_DEMO;

}

void CGEEngine::setup() {
	// Initialise fields
	_lastFrame = 0;

	// Create debugger console
	_console = new CGEConsole(this);

	// Initialise classes that have static members
	VGA::init();
	VFILE::init();
	Bitmap::init();
	TALK::init();

	// Initialise engine objects
	Text = new TEXT(this, progName(), 128);
	Vga = new VGA(M13H);
	_heart = new Heart;
	Hero = new WALK(this, NULL);
	Sys = new SYSTEM(this);
	_pocLight = new Sprite(this, LI);
	for (int i = 0; i < POCKET_NX; i++)
		_pocket[i] = new Sprite(this, NULL);
	_sprite = new Sprite(this, NULL);
	_miniCave = new Sprite(this, NULL);
	_shadow = new Sprite(this, NULL);
	_horzLine = new Sprite(this, HL);
	InfoLine = new INFO_LINE(this, INFO_W);
	_cavLight = new Sprite(this, PR);
	DebugLine = new INFO_LINE(this, SCR_WID);
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
	OffUseCount = atoi(Text->getText(OFF_USE_COUNT));
}

CGEEngine::~CGEEngine() {
	debug("CGEEngine::~CGEEngine");

	// Call classes with static members to clear them up
	TALK::deinit();
	Bitmap::deinit();
	VFILE::deinit();
	VGA::deinit();

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

	_console = new CGEConsole(this);

	// Delete engine objects 
	delete _sprite;
	delete _miniCave;
	delete _shadow;
	delete _horzLine;
	delete InfoLine;
	delete _cavLight;
	delete DebugLine;
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
	delete Text;
	delete _heart;
	delete _pocLight;
	delete _keyboard;
	delete _mouse;
	for (int i = 0; i < POCKET_NX; i++)
		delete _pocket[i];
	delete _snail;
	delete _snail_;
	delete Hero;
	delete Vga;
	delete Sys;
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
