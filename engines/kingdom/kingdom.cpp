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
#include "common/events.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

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
	_MouseValue = 0;
	_CurrMap = 0;
	_StatPlay = 0;
	_SoundNumber = -1;

	_kingartEntries = nullptr;
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
	LoadKingArt();
}

void KingdomGame::InitTools() {
	InitMouse();
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
		_quit = true;
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

void KingdomGame::LoadKingArt() {
	LoadAResource(0x97);
	Common::SeekableReadStream *kingartStream = _RezPointers[0x97];
	int val = kingartStream->readUint32LE();
	int size = val / 4;
	uint32 *kingartIdx = new uint32[size + 1];
	_kingartEntries = new KingArtEntry[size];
	kingartIdx[0] = val;
	for (int i = 1; i < size; i++)
		kingartIdx[i] = kingartStream->readUint32LE();
	kingartIdx[size] = kingartStream->size();

	for (int i = 0; i < size; i++) {
		int chunkSize = kingartIdx[i + 1] - kingartIdx[i];
		_kingartEntries[i].Width = kingartStream->readByte();
		_kingartEntries[i].Height = kingartStream->readByte();

		assert(_kingartEntries[i].Width * _kingartEntries[i].Height == chunkSize - 2);

		_kingartEntries[i].data = new byte[chunkSize - 2];
		kingartStream->read(_kingartEntries[i].data, chunkSize - 2);
	}

	delete[] kingartIdx;
}

void KingdomGame::LoadAResource(int reznum) {
	Common::String path = Common::String(_RezNames[reznum]);
	path.toUppercase();

	debug("Loading resource: %i (%s)\n", reznum, path.c_str());

	if(!_RezSize[reznum]) {
		Common::File *file = new Common::File();
		if(!file->open(path))
			error("Failed to open %s", path.c_str());

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

	const byte *data = (const byte *)surface->getPixels();
	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = 0; curX < 320; curX++) {
		for (uint curY = 0; curY < 200; curY++) {
			const byte *src = data + (curY * 320) + curX;
			byte *dst = (byte *)screen->getBasePtr(curX, curY);
			if (*src != 0xFF)
				*dst = *src;
		}
	}
	g_system->unlockScreen();
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
	InitMouse();
	SetCursor(0x19C / 4);
	_CursorDrawn = false;
	DrawCursor();
}

void KingdomGame::InitMouse() {
	_CursorActive = true;
}

void KingdomGame::SetMouse() {
	g_system->warpMouse(272, 157);
	_CursorX = 272;
	_CursorY = 157;
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
	_ASMode = false;
	_CurrMap = _ASMap;
	_TreeLeftSta = _OldTLS;
	_TreeRightSta = _OldTRS;
	_Pouch = _OldPouch;
	_Help = _OldHelp;
	_IconsClosed = _OldIconsClosed;
}

void KingdomGame::DrawHelpScreen() {
	int picNum;

	switch(_Health) {
	case 2:
		picNum = 166;
		break;
	case 4:
		picNum = 165;
		break;
	case 6:
		picNum = 164;
		break;
	case 8:
		picNum = 163;
		break;
	case 10:
		picNum = 162;
		break;
	case 12:
	default:
		picNum = 161;
		break;
	}

	if (_NoMusic)
		picNum += 6;

	ShowPic(picNum);
}

void KingdomGame::DrawRect(uint minX, uint minY, uint maxX, uint maxY, int color) {
	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = minX; curX < maxX; curX++) {
		for (uint curY = minY; curY < maxY; curY++) {
			byte *dst = (byte *)screen->getBasePtr(curX, curY);
			*dst = color;
		}
	}
	g_system->unlockScreen();
	g_system->updateScreen();
}

void KingdomGame::DrawInventory() {
	FShowPic(108);
	if (word_2D77E == 1 || word_2D7CC == 1 || _ItemInhibit)
		return;

	if (_Inventory[0] > 0)
		DrawIcon(136, 102, 180);

	if (_Inventory[1] > 0)
		DrawIcon(73, 65, 175);

	if (_Inventory[2] > 0)
		DrawIcon(171, 96, 179);

	if (_Inventory[3] > 0)
		DrawIcon(120, 34, 174);

	if (_Inventory[4] > 0)
		DrawIcon(160, 41, 177);

	if (_Inventory[5] > 0)
		DrawIcon(21, 124, 184);

	if (_Inventory[6] > 0)
		DrawIcon(201, 42, 178);

	if (_Inventory[7] > 0)
		DrawIcon(76, 119, 186);

	if (_Inventory[8] > 0)
		DrawIcon(18, 31, 170);

	if (_Inventory[9] > 0)
		DrawIcon(57, 88, 185);

	if (_Inventory[10] > 0)
		DrawIcon(182, 124, 181);

	if (_Inventory[11] > 0)
		DrawIcon(176, 26, 183);

	if (_Inventory[12] > 0)
		DrawIcon(54, 23, 171);

	if (_Inventory[13] > 0)
		DrawIcon(120, 133, 182);

	if (_Inventory[14] > 0)
		DrawIcon(94, 92, 187);

	if (_Inventory[15] > 0)
		DrawIcon(135, 67, 176);

	if (_Inventory[16] > 0)
		DrawIcon(84, 30, 173);

	if (_Inventory[17] > 0)
		DrawIcon(20, 78, 172);

	if (_Inventory[0] > 0)
		DrawIcon(158, 117, 134 + _Inventory[0]);

	if (_Inventory[1] > 0)
		DrawIcon(94, 67, 134 + _Inventory[1]);

	if (_Inventory[2] > 0)
		DrawIcon(193, 105, 134 + _Inventory[2]);

	if (_Inventory[3] > 0)
		DrawIcon(131, 39, 134 + _Inventory[3]);
}

void KingdomGame::SaveGame() {
	debug("STUB: SaveGame");
}

void KingdomGame::PlaySound(int idx) {
	if (idx > 43 || _SoundNumber == idx)
		return;

	// Stop Sound
	if (_mixer->isSoundHandleActive(_soundHandle)) {
		_mixer->stopHandle(_soundHandle);
		ReleaseAResource(idx);
	}

	_SoundNumber = idx;
	if (_SoundNumber == 0 || _NoMusic)
		return;

	int realIdx = _SoundNumber + 200; // Or +250, depending in the original on the sound card
	debug("PlaySound %d : %s", idx, _RezNames[realIdx]);
	LoadAResource(realIdx);

	Common::SeekableReadStream *soundStream = _RezPointers[realIdx];
	Audio::RewindableAudioStream *rewindableStream = Audio::makeRawStream(soundStream, 22050, Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::NO);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, Audio::Mixer::kMaxMixerVolume);
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, rewindableStream);

//	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(rewindableStream, false);
//	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, stream);
}

void KingdomGame::EraseCursor() {
	debug("STUB: EraseCursor");
}

void KingdomGame::ReadMouse() {
	_MouseButton = 0;

	if (_CursorActive) {
		Common::Event event;
		g_system->getEventManager()->pollEvent(event);
		if (event.type == Common::EVENT_MOUSEMOVE) {
		_CursorX = event.mouse.x;
		_CursorY = event.mouse.y;
		}
		if (event.type == Common::EVENT_LBUTTONUP)
			_MouseButton |= 1;
		if (event.type == Common::EVENT_RBUTTONUP)
			_MouseButton |= 2;

		g_system->getEventManager()->pushEvent(event);
	}
}

void KingdomGame::GetUserInput() {
	// CHECKME: _QuitFlag != 0
	if (_quit)
		return;

	_UserInput = WaitKey();

	if (_quit)
		return;

	if (_UserInput == 2 && _Eye)
		_UserInput = _ASMode ? 0x43B : 0x43A;

	if (_UserInput == 1)
		_UserInput = _MouseValue;

	if (_UserInput == 0x2F5) {
		_StatPlay = 600;
		_LoopFlag = true;
	}

	if (_UserInput == 0x42B && _StatPlay == 53 && _GameMode == 0) {
		_OldStatPlay = _StatPlay;
		_StatPlay = 900;
		_LoopFlag = true;
	}

	if (_UserInput == 0x12D && _CurrMap == 1)
		// CHECKME: _quitFlag = 2;
		_quit = true;
}

void KingdomGame::EraseCursorAsm() {
	debug("STUB: EraseCursorAsm");
}

void KingdomGame::RestoreGame() {
	debug("STUB: RestoreGame");
}

void KingdomGame::DrawLocation() {
	if (_DaelonCntr > 0)
		_DaelonCntr--;

	PlaySound(0);
	_IconsClosed = true;
	_TSIconOnly = false;
	_ATimer = 0;
	_ATimerFlag = false;

	int emlValue = _EMLTable[_NodeNum];
	if (emlValue > 0)
		EnAll();

	if (!_MapEx || !emlValue || _Resurrect) {
		if (_StatPlay != 50)
			_Resurrect = false;
		_IconsClosed = false;
	} else {
		_MapEx = false;
		SaveAS();
		FShowPic(emlValue);
		_BTimer = 16;
		while(_BTimer) {
			RefreshSound();
			CheckMainScreen();
		}
		FadeToBlack1();
		DrawRect(4, 17, 228, 161, 0);
		_IconsClosed = false;
		_TSIconOnly = false;
	}
}

void KingdomGame::ProcessMap(int mapNum, int zoom) {
	int var6 = _ZoomTable[mapNum][zoom][0];
	if (!_ASMode)
		SwitchAtoM();
	FShowPic(var6);
	_CurrMap = _ZoomTable[mapNum][zoom][1];

	if (zoom > 0)
		_TreeLeftSta = _ZoomTable[mapNum][zoom - 1][0] == 0 ? 0 : 3;
	else
		_TreeLeftSta = 0;

	if (zoom < 8)
		_TreeRightSta = _ZoomTable[mapNum][zoom + 1][0] == 0 ? 0 : 2;
	else
		_TreeRightSta = 0;
}

void KingdomGame::ProcessMapInput(int mapNum) {
	switch(_UserInput) {
	case 0x43B:
	case 0x443:
		SwitchMtoA();
		_MapStat = 0;
		_StatPlay--;
		break;
	case 0x43F:
		if (_TreeLeftSta == 3) {
			_Zoom--;
			ProcessMap(mapNum, _Zoom);
		} else
			_UserInput = 0;
		break;
	case 0x440:
		if (_TreeRightSta == 2) {
			_Zoom++;
			ProcessMap(mapNum, _Zoom);
		} else
			_UserInput = 0;
		break;
	default:
		if (_UserInput > 0x3FF && _UserInput < 0x428) {
			_StatPlay = _MapExit[_UserInput - 0x400];
			_MapEx = true;
			_LoopFlag = true;
			SwitchAS();
		}

		if (_UserInput > 0x440) {
			SwitchMtoA();
			_MapStat = false;
			_StatPlay--;
			_LoopFlag = true;
		}
		break;
	}
}

void KingdomGame::InventoryDel(int item) {
	if (_Inventory[item] > 0)
		_Inventory[item]--;
}

void KingdomGame::InventoryAdd(int item) {
	if (item >= 4)
		_Inventory[item] = 1;
	else
		_Inventory[item] = 3;
}

void KingdomGame::DrawPic(int reznum) {
	EraseCursor();
	LoadAResource(reznum);

	Image::IFFDecoder decoder;
	if (!decoder.loadStream(*_RezPointers[reznum]))
		return;

	const Graphics::Surface *surface = decoder.getSurface();

	const byte *data = (const byte *)surface->getPixels();
	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = 0; curX < 320; curX++) {
		for (uint curY = 0; curY < 200; curY++) {
			const byte *src = data + (curY * 320) + curX;
			byte *dst = (byte *)screen->getBasePtr(curX, curY);
			if (*src != 0xFF)
				*dst = *src;
		}
	}
	g_system->unlockScreen();
	g_system->updateScreen();

	ReleaseAResource(reznum);
}

void KingdomGame::DisplayIcon(int reznum) {
	PlaySound(0);
	PlaySound(30);
	SaveAS();
	FShowPic(reznum);
	_BTimer = 76;
	ReadMouse();
	
	while(_BTimer != 0 && _MouseButton == 0) {
		RefreshSound();
		ReadMouse();
	}

	FadeToBlack1();
	DrawRect(4, 17, 228, 161, 0);
	RestoreAS();
}

void KingdomGame::SetATimer() {
	_ATimerFlag = true;
	_ATimer = 0;
	int wrkNodeNum = _NodeNum;
	if (word_2D77E == 1 || word_2D7CC == 1)
		return;

	if (_TSIconOnly != 0)
		wrkNodeNum = 79;

	if (_NodeNum == 56 && _Inventory[8] < 1 && _Wizard)
		wrkNodeNum = 80;
	
	for (int i = 0; i < 7; i++) {
		int idx = _IconActTable[wrkNodeNum][i];
		if (_Inventory[idx] > 0) {
			_ATimerFlag = false;
			_ATimer = _Wizard ? 114 : 133;
			PlaySound(0);
			PlaySound(34);
			break;
		}
	}
}

bool KingdomGame::Wound() {
	bool retval = false;
	if (_Health == 12 || _Health == 8 || _Health == 4) {
		_Health -= 2;
		retval = true;
	}
	return retval;
}

void KingdomGame::RefreshSound() {
//	debug("STUB: RefreshSound");
}

void KingdomGame::IncreaseHealth() {
	if (_Health <= 3)
		_Health = 4;
	else if (_Health <= 7)
		_Health = 8;
	else
		_Health = 12;
}

void KingdomGame::CheckSaveGame() {
	debug("STUB: CheckSaveGame");
}

void KingdomGame::CheckMainScreen() {
	if (!_CTimerFlag || _StatPlay == 900 || _StatPlay == 901)
		return;

	_CTimerFlag = false;
	if (_NoIFScreen)
		return;

	if (_HealthOld != _Health) {
		if (_HealthTmr > 0)
			_HealthTmr--;
		else {
			if (_Health <= _HealthOld)
				_HealthOld--;
			else
				_HealthOld++;

			int iconIndex;
			if (_HealthOld == 0)
				iconIndex = 12 - 1;
			else
				iconIndex = 12 - _HealthOld;

			DrawIcon(4, 0, iconIndex);
			_HealthTmr = 1;
		}
	}

	if (_IconRedraw) {
		_IconRedraw = false;
		DrawIcon(4, 0, 12 - _HealthOld);
		DrawIcon(11, 178, _IconPic[0]);
		DrawIcon(38, 178, _IconPic[1]);
		DrawIcon(65, 178, _IconPic[2]);
		DrawIcon(92, 178, _IconPic[3]);
		DrawIcon(119, 178, _IconPic[4]);
		DrawIcon(146, 178, _IconPic[5]);
		DrawIcon(173, 178, _IconPic[6]);
		_TreeLeftPic = 0;
		_TreeRightPic = 0;
		_TreeEyeTimer = 0;
		if (_SkylarTimer != 0 || _ATimer != 0) {
			_TreeHGTimer = 0;
			_TreeHGUPic = 0;
		}
		if (_TideCntl)
			DrawPic(178);
	}

	for (int i = 0; i < 7; i++) {
		int wrkNodeNum = _NodeNum;
		if (_TSIconOnly)
			wrkNodeNum = 79;
		if (_NodeNum == 56 && _Inventory[8] < 1 && _Wizard)
			wrkNodeNum = 80;
		if (_NodeNum == 21 && word_2D770 == 9)
			wrkNodeNum = 81;
		int idx = _IconActTable[wrkNodeNum][i];

		if (_Inventory[idx] >= 1 && word_2D77E != 1 && word_2D7CC != 1 && !_ItemInhibit && !_IconsClosed) {
			if (_IconPic[i] != 12 + idx) {
				if (_IconPic[i] == 89 + i)
					_IconPic[i] = 96 + i;
				else if (_IconPic[i] == 96 + i)
					_IconPic[i] = 31;
				else if (_IconPic[i] == 31)
					_IconPic[i] = 32;
				else if (_IconPic[i] == 32)
					_IconPic[i] = 12 + idx;
				else
					_IconPic[i] = 89 + i;
			} 
		} else if (_IconSel != i && _IconPic[i] != 89 + i) {
			if (_IconPic[i] != 12 + idx)
				_IconPic[i] = 32;
			else if (_IconPic[i] == 32)
				_IconPic[i] = 31;
			else if (_IconPic[i] == 31)
				_IconPic[i] = 96 + i;
			else if (_IconPic[i] == 96 + i)
				_IconPic[i] = 32;
			else
				_IconPic[i] = 89 + i;
		} else
			continue;

		int posX = (27 * i) + 11;
		DrawIcon(posX, 178, _IconPic[i]);
	}

	switch (_TreeLeftSta) {
	case 0:
		if (_TreeLeftPic != 33) {
			DrawIcon(243, 141, 33);
			_TreeLeftPic = 33;
		}
		break;
	case 1:
		if (_TreeLeftPic != 34) {
			DrawIcon(243, 141, 34);
			_TreeLeftPic = 34;
		}
		break;
	case 2:
		if (!_Replay) {
			if (_TreeLeftPic != 33) {
				DrawIcon(243, 141, 33);
				_TreeLeftPic = 33;
			}
		} else if (_TreeLeftPic != 35) {
			DrawIcon(243, 141, 35);
			_TreeLeftPic = 35;
		}
		break;
	case 3:
		if (_TreeLeftPic != 36) {
			DrawIcon(243, 141, 36);
			_TreeLeftPic = 36;
		}
		break;
	default:
		_TreeLeftPic = 33;
		_TreeLeftSta = 0;
		DrawIcon(243, 141, 33);
		break;
	}

	switch (_TreeRightSta) {
	case 0:
		if (_TreeRightPic == 37) {
			DrawIcon(290, 143, 37);
			_TreeRightPic = 37;
		}
		break;
	case 1:
		if (_Help) {
			if (_TreeRightPic != 38) {
				DrawIcon(290, 143, 38);
				_TreeRightPic = 38;
			}
		} else if (_TreeRightPic != 37) {
			DrawIcon(290, 143, 37);
			_TreeRightPic = 37;
		}
		break;
	case 2:
		if (_TreeRightPic != 39) {
			DrawIcon(290, 143, 39);
			_TreeRightPic = 39;
		}
		break;
	default:
		_TreeRightPic = 37;
		_TreeRightSta = 0;
		DrawIcon(290, 143, 37);
		break;
	}

	if (_Eye) {
		if (_TreeEyeTimer == 0) {
			_TreeEyePic = _TEASeq[_TreeEyeSta][0];
			DrawIcon(261, 51, _TreeEyePic);
			_TreeEyeTimer = _TEASeq[_TreeEyeSta][1];
			_TreeEyeSta++;
			if (_TreeEyeSta == 5)
				_TreeEyeSta = 0;
		} else
			_TreeEyeTimer--;
	} else if (_TreeEyePic != 37) {
		DrawIcon(261, 51, 146);
		_TreeEyePic = 37;
		_TreeEyeSta = 0;
		_TreeEyeTimer = 0;
	}

	int timer = 0;
	int delta = 7; // CHECKME: the variable is the same than the one used for the first for(), and the value should therefore be 7  
	if (_SkylarTimer != 0) {
		delta = 772;
		timer = _SkylarTimer;
	}
	if (_ATimer != 0) {
		delta = 19;
		timer = _ATimer;
	}

	if (timer == 0) {
		if (_TreeHGUPic != 147) {
			EraseCursor();
			DrawIcon(249, 171, 147);
			_TreeHGUPic = 147;
		}
	} else if (_TreeHGTimer == 0) {
		_TreeHGPic = _HGASeq[_TreeHGSta][0];
		DrawIcon(249, 185, _TreeHGPic);
		_TreeHGTimer = _HGASeq[_TreeHGSta][1];
		_TreeHGSta++;
		if (_TreeHGSta > 3)
			_TreeHGSta = 0;

		int var2 = 6;
		while (true) {
			if (timer <= 1)
				break;

			timer -= delta;
			if (timer > 1)
				var2--;
			else {
				DrawIcon(249, 171, 40 + var2);
				_TreeHGUPic = 40 + var2;
			}
		}
	} else
		_TreeHGTimer--;
}

bool KingdomGame::ChkDesertObstacles() {
	if (!_Wizard)
		return false;

	_NextNode = _NodeNum;
	if (_LastObs) {
		_LastObs = false;
		return false;
	}

	if (word_2D77E || _rnd->getRandomNumber(6) == 0) {
		if (!word_2D7A6 || _RobberyNode != _NodeNum) {
			if (_LastObstacle != _NodeNum) {
				if (_rnd->getRandomNumber(5) == 0) {
					_StatPlay = 250;
					_LastObstacle = _NodeNum;
					_LastObs = true;
					_LoopFlag = true;
					return true;
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else {
			_StatPlay = 490;
			_LoopFlag = true;
			return true;
		}
	} else {
		_StatPlay = 280;
		_RobberyNode = _NodeNum;
		_LastObstacle = _NodeNum;
		_LastObs = true;
		_LoopFlag = true;
		return true;
	}
}

void KingdomGame::SwitchAtoM() {
	_ASMode = true;
	_ASMap = _CurrMap;
	SaveAS();
	_IconSel = 9;
	_OldTLS = _TreeLeftSta;
	_OldTRS = _TreeRightSta;
	_OldPouch = _Pouch;
	_OldHelp = _Help;
	_OldIconsClosed = _IconsClosed;
	_TreeLeftSta = 0;
	_TreeRightSta = 0;
	_Pouch = false;
	_Help = false;
	_IconsClosed = true;
}

void KingdomGame::SwitchMtoA() {
	SwitchAS();
	FadeToBlack1();
	DrawRect(4, 17, 228, 161, 0);
	RestoreAS();
}

void KingdomGame::DrawIcon(int x, int y, int index) {
	debug("STUB: DrawIcon");
}

int KingdomGame::GetAKey() {
	DrawCursor();
	if (_MouseButton != 0 && _MouseDebound == false) {
		_MouseDebound = true;
		return (_MouseButton & 2) ? 2 : 1;
	}

	int retval = 0;
	_MouseDebound = false;
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
			// if keyboard used, retVal = getch() + 0x100
			if (!event.kbd.hasFlags(Common::KBD_CTRL) && !event.kbd.hasFlags(Common::KBD_ALT))
				retval = 0x100 + event.kbd.keycode;
			else if (event.kbd.keycode == Common::KEYCODE_d && event.kbd.hasFlags(Common::KBD_CTRL))
				_console->attach();
			break;
		case Common::EVENT_LBUTTONUP: // retval == 2?
			if (_Eye)
				retval = !_ASMode ? 0x43A : 0x43B;
			break;
		case Common::EVENT_RBUTTONUP: // retval == 1?
			retval = _MouseValue;
			break;
		case Common::EVENT_MOUSEMOVE:
			_CursorX = event.mouse.x;
			_CursorY = event.mouse.y;

		default:
			RefreshSound();
			CheckMainScreen();
			if (_ATimerFlag) {
				_ATimerFlag = false;
				retval = 0x2F1;
			} else if (_BTimerFlag) {
				_BTimerFlag = false;
				retval = 0x2F2;
			} else if (_CTimerFlag) {
				_CTimerFlag = false;
				retval = 0x2F5;
			} else
				retval= 0;
			break;
		}
	}
	return retval;
}

int KingdomGame::WaitKey() {
	return GetAKey();
}

void KingdomGame::DrawCursor() {
	ReadMouse();

	CursorType();
	SetCursor(_CursorDef);
	_OldCursorX = _CursorX;
	_OldCursorY = _CursorY;
	_OldCursorDef = _CursorDef;

	CursorMan.showMouse(true);
	_CursorDrawn = true;
}

void KingdomGame::CursorType() {
	_MouseValue = 0;
	if (_CurrMap != 1 && _StatPlay >= 30) {
		int var2 = _StatPlay == 901 ? 16 : 0;
		int var6 = _StatPlay == 901 ? 35 : 16;
		for (int i = 0; i < var6 + 1; i++) {
			if (i == var6) {
				int tmpVal = checkMouseMapAS();
				if (tmpVal == -1) {
					CursorTypeExit();
					return;
				} else
					_MouseValue = tmpVal;
			} else if (_CursorX >= _MouseMapMS[var2 + i]._minX && _CursorX < _MouseMapMS[var2 + i]._maxX && _CursorY >= _MouseMapMS[var2 + i]._minY && _CursorY < _MouseMapMS[var2 + i]._maxY) {
				_MouseValue = _MouseMapMS[var2 + i]._mouseValue;
				break;
			}
		}
	} else {
		int tmpVal = checkMouseMapAS();
		if (tmpVal == -1) {
			CursorTypeExit();
			return;
		} else {
			_MouseValue = tmpVal;
		}
	}

	switch(_MouseValue) {
	case 0x18A:
		if (_Eye)
			_MouseValue = _ASMode == 0 ? 0x43A : 0x43B;
		else
			_MouseValue = 0;
		break;
	case 0x18C:
		if (_TreeLeftSta == 1)
			_MouseValue = 0x43D;
		else if (_TreeLeftSta == 3)
			_MouseValue = 0x43F;
		else if (_TreeLeftSta == 0)
			_MouseValue = 0;
		else if (_TreeLeftSta == 2 && _Replay)
			_MouseValue = 0x43E;
		else
			_MouseValue = 0;
		break;
	case 0x18D:
		if (_TreeRightSta == 1)
			_MouseValue = _Help ? 0x43C : 0;
		if (_TreeRightSta == 2)
			_MouseValue = 0x440;
		break;
	case 0x24A:
		if (_SaveFile == 0)
			_MouseValue = 0;
		break;
	case 0x407:
		if (_StatPlay == 182 && word_2D76A < 9)
			_MouseValue = 0;
		break;
	case 0x40D:
		if (word_2D77E == 1)
			_MouseValue = 0;
		break;
	case 0x41F:
		if (word_2D784 == 0)
			_MouseValue = 0;
		break;
	case 0x422:
	case 0x425:
		if (!_Wizard)
			_MouseValue = 0;
		break;
	case 0x428:
		if (_NodeNum == 5 && _GameMode != 2 && _Spell1)
			_MouseValue = 0;
		break;
	case 0x42A:
		if (_NodeNum == 5 && _GameMode != 2 && _Spell2)
			_MouseValue = 0;
		break;
	case 0x42B:
		if (_NodeNum == 5 && _GameMode != 2 && _Spell3)
			_MouseValue = 0;
		break;
	case 0x445:
		if (_StatPlay == 161 && word_2D766 == 0 && _Wizard)
			_MouseValue = 0x450;
		break;
	case 0x44F:
		if (!_Pouch)
			_MouseValue = 0;
		break;
	case 0x457:
		if (!_TideCntl)
			_MouseValue = 0;
		break;
	}

	_IconSelect = 9;
	for (int var6 = 0; var6 < 8; var6++) {
		if (_MouseValue == 181 + var6) {
			int var2 = _NodeNum;
			if (_TSIconOnly!= 0)
				var2 = 79;
			if (_NodeNum == 56 && _Inventory[8] < 1 && _Wizard)
				var2 = 80;
			int indx = _IconActTable[var2][var6];
			if (_Inventory[indx] != 0 && word_2D77E != 1 && word_2D7CC != 1 && !_IconsClosed && !_ItemInhibit) {
				_MouseValue = indx + 0x428;
				_IconSelect = var6;
				break;
			}
			_MouseValue = 0;
		}
	}

	if (_CurrMap == 11) {
		if (_MouseValue > 0x427 && _MouseValue < 0x43A) {
			if (_Inventory[_MouseValue - 0x428] < 1)
				_MouseValue = 0x241;
		}
	}
	CursorTypeExit();
}

void KingdomGame::CursorTypeExit() {
	if (_MouseValue >= 0x400)
		_CursorDef = _CursorTable[_MouseValue - 0x400];
	else 
		_CursorDef = (_MouseValue != 0) ? 0x68 : 0x67;
}

int KingdomGame::checkMouseMapAS() {
	for (int i = 0; i < 16; i++) {
		if (_CursorX >= _MouseMapAS[_CurrMap][i]._minX && _CursorX < _MouseMapAS[_CurrMap][i]._maxX
			&& _CursorY >= _MouseMapAS[_CurrMap][i]._minY && _CursorY < _MouseMapAS[_CurrMap][i]._maxY)
			return _MouseMapAS[_CurrMap][i]._mouseValue;
	}
	if (_CurrMap == 11) {
		for (int i = 0; i < 16; i++) {
			if (_CursorX >= _MouseMapAS[12][i]._minX && _CursorX < _MouseMapAS[12][i]._maxX
				&& _CursorY >= _MouseMapAS[12][i]._minY && _CursorY < _MouseMapAS[12][i]._maxY)
				return _MouseMapAS[12][i]._mouseValue;
		}
	}
	return -1;
}
void KingdomGame::SetCursor(int cursor) {
	KingArtEntry Cursor = _kingartEntries[cursor];
	CursorMan.replaceCursor(Cursor.data, Cursor.Width, Cursor.Height, 0, 0, 255);
}

} // End of namespace Kingdom
