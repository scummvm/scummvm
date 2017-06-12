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
#include "image/iff.h"
#include "engines/util.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/stream.h"
#include "common/memstream.h"

#include "kingdom/kingdom.h"
#include "kingdom/reznames.h"

namespace Kingdom {

KingdomGame::KingdomGame(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_console = nullptr;
	_rnd = new Common::RandomSource("kingdom");

	DebugMan.addDebugChannel(kDebugGeneral, "general", "General debug level");
	for (int i = 0; i < 509; i++) {
		_RezPointers[i] = nullptr;
		_RezSize[i] = 0;
	}
}

KingdomGame::~KingdomGame() {
	delete _console;
	delete _rnd;
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
	// Load Pics\kingArt.art
	LoadAResource(0x97);
	_ArtPtr = _RezPointers[0x97 - 1];
}

void KingdomGame::InitTools() {
	debug("STUB: InitTools");
	//CHECKME: InitTimers?
	ShowPic(124);
	InitCursor();
	SetMouse();
	FadeToBlack2();
	InitMPlayer();
}

void KingdomGame::TitlePage() {
	if (shouldQuit())
		return;

	_FstFwd = true;
	_NoIFScreen = true;
	_Sound = false;
	FadeToBlack2();
	PlayMovie(200);
	FadeToBlack2();
	PlayMovie(206);
	FadeToBlack2();
	PlayMovie(198);
	FadeToBlack2();
}

void KingdomGame::InitPlay() {
	memset(_Inventory, 0xFF, 19);

	//TODO: Init game flags, once the hardcoded logic is implemented and they are renamed

	for (int i = 0; i < 7; i++)
		_IconPic[i] = 89 + i;

	_FrameStop = 0;
	_GameMode = 0;
	_DaelonCntr = 0;
	_StatPlay = 10;
	_Spell1 = false;
	_Spell2 = false;
	_Spell3 = false;
	_ItemInhibit = false;
	_ASMode = false;
	_ATimerFlag = false;
	_BTimerFlag = false;
	_CTimerFlag = false;
	_SkylarTimerFlag = false;
	_ATimer = 0;
	_BTimer = 0;
	_CTimer = 0;
	_SkylarTimer = 0;
	_TideCntl = false;
	_MapEx = false;
	_Health = 12;
	_HealthOld = 1;
	_HealthTmr = 0;
	_TreeEyeTimer = 0;
	_TreeHGTimer = 0;
	_TreeHGUPic = 147;
	_TreeLeftPic = 0;
	_TreeRightPic = 0;
	_TreeRightSta = 1;
	_TSIconOnly = false;
	_LastObs = false;
	EnAll();
	_Pouch = true;
	_NoIFScreen = true;
	_NoMusic = false;
	_FstFwd = true;
	if (_ASPtr)
		free(_ASPtr);

	_ASPtr = nullptr;
}

void KingdomGame::InitHelp() {
	_GameMode = 0;
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

void KingdomGame::LoadAResource(int reznum) {
	// CHECKME: Weird off-by-one here?
	reznum--;

	Common::String path = Common::String(_RezNames[reznum]);
	path.toUppercase();

	debug("Loading resource: %i (%s)\n", reznum, path.c_str());

	if(!_RezSize[reznum]) {
		Common::File *file = new Common::File();
		if(!file->open(path))
			error("Failed to open %s", path);

		_RezSize[reznum] = file->size();
		file->seek(0, SEEK_SET);
		_RezPointers[reznum] = file->readStream(_RezSize[reznum]);
		file->close();
		delete file;
	}
}

void KingdomGame::ShowPic(int reznum) {
	debug("STUB ShowPic %i\n", reznum);
	LoadAResource(reznum);
	Image::IFFDecoder decoder;
	if (!decoder.loadStream(*_RezPointers[reznum - 1]))
		return;

	const byte *palette = decoder.getPalette();
	int paletteColorCount = decoder.getPaletteColorCount();
	g_system->getPaletteManager()->setPalette(palette, 0, paletteColorCount);

	const Graphics::Surface *surface = decoder.getSurface();
	g_system->copyRectToScreen(surface->getPixels(), 320, 0, 0, 320, 200);
	g_system->updateScreen();
}

void KingdomGame::InitCursor() {
	debug("STUB: InitCursor");
}

void KingdomGame::SetMouse() {
	debug("STUB: SetMouse");
}

void KingdomGame::InitMPlayer() {
	debug("STUB: InitMPlayer");
}

void KingdomGame::PlayMovie(int movieNum) {
	debug("STUB: PlayMovie");
}

void KingdomGame::EnAll() {
	debug("STUB: EnAll");
}

} // End of namespace Kingdom
