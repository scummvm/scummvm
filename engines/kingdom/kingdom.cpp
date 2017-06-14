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

namespace Kingdom {

KingdomGame::KingdomGame(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_console = nullptr;
	_rnd = new Common::RandomSource("kingdom");

	DebugMan.addDebugChannel(kDebugGeneral, "general", "General debug level");
	for (int i = 0; i < 510; i++) {
		_RezPointers[i] = nullptr;
		_RezSize[i] = 0;
	}

	_ASPtr = nullptr;
	_quit = false;
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

	while (!_quit) {
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

		if (!_LoopFlag)
			GetUserInput();

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
	_ArtPtr = _RezPointers[0x97];
}

void KingdomGame::InitTools() {
	//CHECKME: InitTimers?
	ShowPic(124);
	InitCursor();
	SetMouse();
	FadeToBlack2();
	InitMPlayer();
}

void KingdomGame::TitlePage() {
	// TODO: Check on QuitFlag == 2
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

void KingdomGame::FadeToBlack1() {
	debug("STUB: FadeToBlack1");
}

void KingdomGame::FadeToBlack2() {
	debug("STUB: FadeToBlack2");
}

void KingdomGame::GameHelp_Sub43C() {
	FadeToBlack1();
	_CurrMap = _ASMap;
	DrawRect(4, 17, 228, 161, 0);
	RestoreAS();
	_UserInput = 0;
	_GameMode = 0;
	_IconsClosed = false;
	_TreeLeftSta = _OldTLS;
	_Eye = _OldEye;
	_Help = _OldHelp;
	_Pouch = _OldPouch;
}

void KingdomGame::GameHelp() {
	if (!_GameMode) {
		if (_UserInput == 0x43C) {
			SaveAS();
			_ASMap = _CurrMap;
			_CurrMap = 0;
			FadeToBlack1();
			DrawRect(4, 17, 228, 161, 0);
			DrawHelpScreen();
			_GameMode = 1;
			_OldTLS = _TreeLeftSta;
			_TreeLeftSta = 0;
			_IconsClosed = true;
			_OldEye = _Eye;
			_Eye = false;
			_OldHelp = _Help;
			_OldPouch = _Pouch;
			_Pouch = false;
			_UserInput = 0;
		}
		if (_UserInput == 0x44F) {
			SaveAS();
			_ASMap = _CurrMap;
			_GameMode = 2;
			_OldTLS = _TreeLeftSta;
			_TreeLeftSta = 0;
			_IconsClosed = true;
			_OldEye = _Eye;
			_Eye = false;
			_OldHelp = _Help;
			_Help = false;
			_OldPouch = _Pouch;
			FadeToBlack1();
			DrawRect(4, 17, 228, 161, 0);
			DrawInventory();

			if (word_2D7CC == 1 || word_2D77E == 1 || _ItemInhibit)
				_CurrMap = 10;
			else
				_CurrMap = 11;

			_UserInput = 0;
		}
	}
	if (_GameMode == 0)
		return;

	switch(_UserInput) {
	case 0x240:
		FadeToBlack2();
		//TODO: Set _quitFlag to 1
		break;
	case 0x241:
		GameHelp_Sub43C();
		return;
		break;
	case 0x242:
		if (_NoMusic) {
			_NoMusic = false;
			PlaySound(1);
		} else {
			_NoMusic = true;
			PlaySound(0);
		}
		DrawHelpScreen();
		break;
	case 0x243: {
		FadeToBlack2();
		_KeyActive = false;
		_NoIFScreen = true;
		PlaySound(0);
		int var = _PMovie;
		while(!_KeyActive) {
			PlayMovie(54);
			FadeToBlack2();
		}
		_PMovie = var;
		_NoIFScreen = false;
		ShowPic(106);
		DrawHelpScreen();
		_IconRedraw = true;
		PlaySound(1);
		}
		break;
	case 0x244:
		//TODO: Set _quitFlag to 2
		break;
	case 0x245: {
		FadeToBlack1();
		int var = _PMovie;
		DrawRect(4, 17, 228, 161, 0);
		PlayMovie(205);
		FadeToBlack1();
		DrawRect(4, 17, 228, 161, 0);
		PlayMovie(199);
		FadeToBlack1();
		DrawRect(4, 17, 228, 161, 0);
		DrawHelpScreen();
		_PMovie = var;
		}
		break;
	case 0x246:
		SaveGame();
		break;
	case 0x43B:
	case 0x43C:
	case 0x44F:
		GameHelp_Sub43C();
		return;
	}

	if (_UserInput > 0x427 && _UserInput < 0x43A)
		FShowPic(130 + _UserInput - 0x428);

	if (_UserInput == 0x260) {
		DrawInventory();
		if (word_2D7CC == 1 || word_2D77E == 1)
			_CurrMap = 10;
		else
			_CurrMap = 11;

		_UserInput = 0;
	}
}

void KingdomGame::GPLogic3() {
	debug("STUB: GPLogic3");
}

void KingdomGame::LoadAResource(int reznum) {
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

void KingdomGame::ReleaseAResource(int reznum) {
	if (_RezSize[reznum]) {
		delete _RezPointers[reznum];
		_RezSize[reznum] = 0;
	}
}

void KingdomGame::ShowPic(int reznum) {
	EraseCursor();

	LoadAResource(reznum);
	Image::IFFDecoder decoder;
	if (!decoder.loadStream(*_RezPointers[reznum]))
		return;

	const byte *palette = decoder.getPalette();
	int paletteColorCount = decoder.getPaletteColorCount();
	g_system->getPaletteManager()->setPalette(palette, 0, paletteColorCount);

	const Graphics::Surface *surface = decoder.getSurface();
	g_system->copyRectToScreen(surface->getPixels(), 320, 0, 0, 320, 200);
	g_system->updateScreen();

	ReleaseAResource(reznum);
}

void KingdomGame::FShowPic(int reznum) {
	EraseCursor();
	FadeToBlack1();
	DrawRect(4, 17, 228, 161, 0);
	ShowPic(reznum);
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
	_Help = true;
	_Eye = true;
	_Replay = true;
	_Pouch = true;
	_FstFwd = true;
}

void KingdomGame::DsAll() {
	_Help = false;
	_Eye = false;
	_Replay = false;
	_Pouch = false;
	_FstFwd = false;
}

void KingdomGame::SaveAS() {
	debug("STUB: SaveAS");
}

void KingdomGame::RestoreAS() {
	debug("STUB: RestoreAS");
}

void KingdomGame::SwitchAS() {
	debug("STUB: SwitchAS");
}

void KingdomGame::DrawHelpScreen() {
	debug("STUB: DrawHelpScreen");
}

void KingdomGame::DrawRect(int v1, int v2, int v3, int v4, int v5) {
	debug("STUB: DrawRect)");
}

void KingdomGame::DrawInventory() {
	debug("STUB: DrawInventory");
}

void KingdomGame::SaveGame() {
	debug("STUB: SaveGame");
}

void KingdomGame::PlaySound(int v1) {
	debug("STUB: PlaySound");
}

void KingdomGame::EraseCursor() {
	debug("STUB: EraseCursor");
}

void KingdomGame::ReadMouse() {
	debug("STUB: ReadMouse");
}

void KingdomGame::RefreshMouse() {
	debug ("STUB: RefreshMouse");
}

void KingdomGame::GetUserInput() {
	debug("STUB: GetUserInput");

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
				case Common::EVENT_QUIT:
				case Common::EVENT_RTL:
					_quit = true;
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

void KingdomGame::EraseCursorAsm() {
	debug("STUB: EraseCursorAsm");
}

void KingdomGame::RestoreGame() {
	debug("STUB: RestoreGame");
}

void KingdomGame::DrawLocation() {
	debug("STUB: DrawLocation");
}

void KingdomGame::ProcessMap(int mapNum, int zoom) {
	debug("STUB: ProcessMap");
}

void KingdomGame::ProcessMapInput(int mapNum) {
	debug("STUB: ProcessMapInput");
}

void KingdomGame::InventoryDel(int item) {
	debug("STUB: InventoryDel");
}

void KingdomGame::InventoryAdd(int item) {
	debug("STUB: InventoryAdd");
}

void KingdomGame::DrawPic(int reznum) {
	debug("STUB: DrawPic");
}

void KingdomGame::DisplayIcon(int reznum) {
	debug("STUB: DisplayIcon");
}

void KingdomGame::SetATimer() {
	debug("STUB: SetATimer");
}

bool KingdomGame::Wound() {
	debug("STUB: Wound");
	return false;
}

void KingdomGame::RefreshSound() {
	debug("STUB: RefreshSound");
}

void KingdomGame::IncreaseHealth() {
	debug("STUB: IncreaseHealth");
}

void KingdomGame::CheckSaveGame() {
	debug("STUB: CheckSaveGame");
}

void KingdomGame::CheckMainScreen() {
	debug("STUB: CheckMainScreen");
}

bool KingdomGame::ChkDesertObstacles() {
	debug("STUB: ChkDesertObstacles");
	return false;
}

void KingdomGame::SwitchMtoA() {
	debug("STUB: SwitchMtoA");
}

void KingdomGame::_DrawIcon(int x, int y, Common::MemoryReadStream icon) {
	debug("STUB: _DrawIcon");
}
} // End of namespace Kingdom
