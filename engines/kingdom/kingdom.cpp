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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/palette.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "common/system.h"
#include "engines/util.h"
#include "common/debug.h"
#include "common/debug-channels.h"

#include "kingdom/kingdom.h"

namespace Kingdom {

KingdomGame::KingdomGame(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_console = nullptr;

	DebugMan.addDebugChannel(kDebugGeneral, "general", "General debug level");
}

KingdomGame::~KingdomGame() {
	delete _console;
}

Common::Error KingdomGame::run() {
	initGraphics(320, 200, false);
	_console = new Console(this);

	SetupPics();
	InitTools();
	TitlePage();

	InitPlay();
	InitHelp();

	bool quit = false;
	while (!quit) {
		_LoopFlag = false;
		GameHelp();
		if (_GameMode == 0) {
			if (_StatPlay < 250)
				GPLogic1();
			if (_StatPlay > 249 && _StatPlay < 500)
				GPLogic2();
			if (_StatPlay > 499 && _StatPlay < 900)
				GPLogic3();
			if (_StatPlay > 899)
				GPLogic4();
		}

		if (!_LoopFlag) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				switch (event.type) {
				case Common::EVENT_QUIT:
				case Common::EVENT_RTL:
					quit = true;
					break;

				case Common::EVENT_LBUTTONDOWN:
					break;
				case Common::EVENT_KEYDOWN:
					if (event.kbd.keycode == Common::KEYCODE_d && event.kbd.hasFlags(Common::KBD_CTRL))
						_console->attach();
					break;
				default:
					break;
				}
			}
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	FadeToBlack2();
	return Common::kNoError;
}

void KingdomGame::drawScreen() {
	//TODO

	_console->onFrame();
}

void KingdomGame::SetupPics() {
	debug("STUB: SetupPics");
}

void KingdomGame::InitTools() {
	debug("STUB: InitTools");
}

void KingdomGame::TitlePage() {
	debug("STUB: TitlePage");
}

void KingdomGame::InitPlay() {
	debug("STUB: InitPlay");
}

void KingdomGame::InitHelp() {
	debug("STUB: InitHelp");
}

void KingdomGame::FadeToBlack2() {
	debug("STUB: FadeToBlack2");
}

void KingdomGame::GameHelp() {
	debug("STUB: GameHelp");
}

void KingdomGame::GPLogic1() {
	debug("STUB: GPLogic1");
}

void KingdomGame::GPLogic2() {
	debug("STUB: GPLogic2");
}

void KingdomGame::GPLogic3() {
	debug("STUB: GPLogic3");
}

void KingdomGame::GPLogic4() {
	debug("STUB: GPLogic4");
}

} // End of namespace Kingdom
