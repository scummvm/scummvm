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
	// Create debugger console
	_console = new CGEConsole(this);

	// Initialise classes that have static members
	VFILE::init();

	// Initialise engine objects
	Text = new TEXT(this, ProgName(), 128);
	Vga = new VGA(M13H);
	Heart = new HEART;
	Hero = new WALK(this, NULL);
	Sys = new SYSTEM(this);
	PocLight = new SPRITE(this, LI);
	Mouse = new MOUSE(this);
	for (int i = 0; i < POCKET_NX; i++)
		Pocket[i] = new SPRITE(this, NULL);
	Sprite = new SPRITE(this, NULL);
	MiniCave = new SPRITE(this, NULL);
	Shadow = new SPRITE(this, NULL);
	HorzLine = new SPRITE(this, HL);
	InfoLine = new INFO_LINE(this, INFO_W);
	CavLight = new SPRITE(this, PR);
	DebugLine = new INFO_LINE(this, SCR_WID);
	MB[0] = new BITMAP("BRICK");
	MB[1] = NULL;
	HL[0] = new BITMAP("HLINE");
	HL[1] = NULL;
	MC[0] = new BITMAP("MOUSE");
	MC[1] = new BITMAP("DUMMY");
	MC[2] = NULL;
	PR[0] = new BITMAP("PRESS");
	PR[1] = NULL;
	SP[0] = new BITMAP("SPK_L");
	SP[1] = new BITMAP("SPK_R");
	SP[2] = NULL;
	LI[0] = new BITMAP("LITE0");
	LI[1] = new BITMAP("LITE1");
	LI[2] = new BITMAP("LITE2");
	LI[3] = new BITMAP("LITE3");
	LI[4] = NULL;
	Snail = new SNAIL(this, false);
	Snail_ = new SNAIL(this, true);

	OffUseCount = atoi(Text->getText(OFF_USE_COUNT));
}

CGEEngine::~CGEEngine() {
	debug("CGEEngine::~CGEEngine");

	// Call classes with static members to clear them up
	VFILE::deinit();

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();

	_console = new CGEConsole(this);

	// Delete engine objects
	delete Text;
	delete Vga;
	delete Heart;
	delete Hero;
	delete Sys;
	delete PocLight;
	delete Mouse;
	for (int i = 0; i < POCKET_NX; i++)
		delete Pocket[i];
	delete Sprite;
	delete MiniCave;
	delete Shadow;
	delete HorzLine;
	delete InfoLine;
	delete CavLight;
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
	delete Snail;
	delete Snail_;
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
