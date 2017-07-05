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
#include "common/translation.h"

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

	for (int i = 0; i < 99; i++)
		_Nodes[i] = 0;

	_ASPtr = nullptr;
	_quit = false;
	_MouseValue = 0;
	_CurrMap = 0;
	_statPlay = 0;
	_SoundNumber = -1;

	_kingartEntries = nullptr;

	_tickCount = 0;
	_oldTime = g_system->getMillis();
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
		_loopFlag = false;
		GameHelp();
		if (_gameMode == 0) {
			if (_statPlay < 250)
				GPLogic1();
			if (_statPlay > 249 && _statPlay < 500)
				GPLogic2();
			if (_statPlay > 499 && _statPlay < 900)
				GPLogic3();
			if (_statPlay > 899)
				GPLogic4();
		}

		if (!_loopFlag)
			GetUserInput();

		refreshScreen();
	}

	FadeToBlack2();
	return Common::kNoError;
}

void KingdomGame::refreshScreen() {
	g_system->updateScreen();
	checkTimers();
}

void KingdomGame::checkTimers() {
	uint32 newTime = g_system->getMillis();
	int32 delay = 11 - (newTime - _oldTime);
	if (delay > 0)
		g_system->delayMillis(delay);
	_oldTime = newTime;
	_tickCount++;

	if (_tickCount == 5) {
		_tickCount = 0;
	} else
		return;

	if (_ATimer != 0) {
		_ATimer--;
		if (_ATimer == 0)
			_ATimerFlag = true;
	}

	if (_BTimer != 0) {
		_BTimer--;
		if (_BTimer == 0)
			_BTimerFlag = true;
	}

	if (_CTimer != 0) {
		_CTimer--;
		if (_CTimer == 0) {
			_CTimerFlag = true;
			_CTimer = 4;
		}
	} else
		_CTimer = 4;

	if (_SkylarTimer != 0) {
		_SkylarTimer--;
		if (_SkylarTimer == 0)
			_SkylarTimerFlag = true;
	}

	_PalStepFlag = false;
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

	_fstFwd = true;
	_noIFScreen = true;
	_sound = false;
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

	for (int i = 0; i < 99; i++)
		_Nodes[i] = 0;

	for (int i = 0; i < 7; i++)
		_IconPic[i] = 89 + i;

	_FrameStop = 0;
	_gameMode = 0;
	_DaelonCntr = 0;
	_statPlay = 10;
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
	_noIFScreen = true;
	_NoMusic = false;
	_fstFwd = true;

	delete[] _ASPtr;
	_ASPtr = nullptr;
}

void KingdomGame::InitHelp() {
	_gameMode = 0;
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
	_gameMode = 0;
	_IconsClosed = false;
	_TreeLeftSta = _OldTLS;
	_Eye = _OldEye;
	_Help = _OldHelp;
	_Pouch = _OldPouch;
}

void KingdomGame::GameHelp() {
	if (!_gameMode) {
		if (_UserInput == 0x43C) {
			SaveAS();
			_ASMap = _CurrMap;
			_CurrMap = 0;
			FadeToBlack1();
			DrawRect(4, 17, 228, 161, 0);
			DrawHelpScreen();
			_gameMode = 1;
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
			_gameMode = 2;
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

			if (_Nodes[68] == 1 || _Nodes[29] == 1 || _ItemInhibit)
				_CurrMap = 10;
			else
				_CurrMap = 11;

			_UserInput = 0;
		}
	}
	if (_gameMode == 0)
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
		_noIFScreen = true;
		PlaySound(0);
		int var = _PMovie;
		while(!_KeyActive) {
			PlayMovie(54);
			FadeToBlack2();
		}
		_PMovie = var;
		_noIFScreen = false;
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
		saveGame();
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
		if (_Nodes[68] == 1 || _Nodes[29] == 1)
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
		_kingartEntries[i]._width = kingartStream->readByte();
		_kingartEntries[i]._height = kingartStream->readByte();

		assert(_kingartEntries[i]._width * _kingartEntries[i]._height == chunkSize - 2);

		_kingartEntries[i]._data = new byte[chunkSize - 2];
		kingartStream->read(_kingartEntries[i]._data, chunkSize - 2);
	}

	delete[] kingartIdx;
}

void KingdomGame::LoadAResource(int reznum) {
	Common::String path = Common::String(_rezNames[reznum]);
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
	// No implementation required
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
	_fstFwd = true;
}

void KingdomGame::DsAll() {
	_Help = false;
	_Eye = false;
	_Replay = false;
	_Pouch = false;
	_fstFwd = false;
}

void KingdomGame::SaveAS() {
	byte palette[256 * 3];
	delete[] _ASPtr;
	_ASPtr = new byte[224 * 146 + 768];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);

	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = 0; curX < 224; curX++) {
		for (uint curY = 0; curY < 146; curY++) {
			byte *ptr = (byte *)screen->getBasePtr(curX + 4, curY + 15);
			_ASPtr[curY * 224 + curX] = *ptr;
		}
	}

	for (uint i = 0; i < 768; i++)
		_ASPtr[224 * 146 + i] = palette[i];

	g_system->unlockScreen();
	g_system->updateScreen();
}

void KingdomGame::RestoreAS() {
	byte palette[256 * 3];
	for (uint i = 0; i < 768; i++)
		palette[i] = _ASPtr[224 * 146 + i];

	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	::Graphics::Surface *screen = g_system->lockScreen();
	for (uint curX = 0; curX < 224; curX++) {
		for (uint curY = 0; curY < 146; curY++) {
			byte *ptr = (byte *)screen->getBasePtr(curX + 4, curY + 15);
			*ptr = _ASPtr[curY * 224 + curX];
		}
	}

	g_system->unlockScreen();
	g_system->updateScreen();
	delete[] _ASPtr;
	_ASPtr = nullptr;
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
	if (_Nodes[29] == 1 || _Nodes[68] == 1 || _ItemInhibit)
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

Common::String KingdomGame::getSavegameFilename(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

void KingdomGame::saveGame() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
	int16 savegameId = dialog->runModalWithCurrentTarget();
	Common::String savegameDescription = dialog->getResultString();
	delete dialog;
	if (savegameId < 0)
		return; // dialog aborted
	saveGameState(savegameId, savegameDescription);
}

void KingdomGame::restoreGame() {
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
	int16 savegameId = dialog->runModalWithCurrentTarget();
	delete dialog;

	if (savegameId < 0)
		return; // dialog aborted

	loadGameState(savegameId);
}

Common::Error KingdomGame::saveGameState(int slot, const Common::String &desc) {
	Common::String savegameFile = getSavegameFilename(slot);
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::OutSaveFile *out = saveMan->openForSaving(savegameFile);

	if (!out)
		return Common::kCreatingFileFailed;

	KingdomSavegameHeader header;
	header._saveName = desc;
	writeSavegameHeader(out, header);

	Common::Serializer s(nullptr, out);
	synchronize(s);

	out->finalize();
	delete out;

	return Common::kNoError;
}

Common::Error KingdomGame::loadGameState(int slot) {
	Common::String savegameFile = getSavegameFilename(slot);
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *inFile = saveMan->openForLoading(savegameFile);
	if (!inFile)
		return Common::kReadingFailed;

	Common::Serializer s(inFile, nullptr);

	KingdomSavegameHeader header;
	if (!readSavegameHeader(inFile, header))
		error("Invalid savegame");

	if (header._thumbnail) {
		header._thumbnail->free();
		delete header._thumbnail;
	}

	// Load most of the savegame data
	synchronize(s);
	delete inFile;

	delete[] _ASPtr;
	_ASPtr = nullptr;

	PlaySound(_SoundNumber);
	for (int i = 0; i < 7; i++)
		_IconPic[i] = 89 + i;
	
	_FrameStop = 0;
	_gameMode = 0;
	_ASMode = false;
	_HealthTmr = 0;
	_noIFScreen = false;
	_IconRedraw = true;
	_TreeRightSta = 1;
	_ATimerFlag = false;
	_ATimer = 0;
	_BTimerFlag = false;
	_BTimer = 0;
	_TreeEyeTimer = 0;
	_TreeEyePic = 0;
	_TreeHGUPic = 0;
	_CursorDrawn = false;
	ShowPic(106);
	_gameMode = 0;
	_IconsClosed = false;
	DrawRect(4, 17, 228, 161, 0);
	_UserInput = 0x43E;
	_loopFlag = true;

	return Common::kNoError;
}

void KingdomGame::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_statPlay);
	s.syncAsSint16LE(_ASMap);
	s.syncAsSint16LE(_DaelonCntr);
	s.syncAsSint16LE(_Health);
	s.syncAsSint16LE(_HealthOld);
	s.syncAsSint16LE(_LastObstacle);
	s.syncAsSint16LE(_NextNode);
	s.syncAsSint16LE(_NodeNum);
	s.syncAsSint16LE(_PMovie);
	s.syncAsSint16LE(_RtnNode);
	s.syncAsSint16LE(_RobberyNode);
	s.syncAsSint16LE(_SoundNumber);
	s.syncAsSint16LE(_TreeEyePic);
	s.syncAsSint16LE(_TreeEyeSta);
	s.syncAsSint16LE(_TreeHGPic);
	s.syncAsSint16LE(_TreeHGSta);
	s.syncAsSint16LE(_OldTLS);
	s.syncAsSint16LE(_CTimer);
	s.syncAsSint16LE(_SkylarTimer);

	for (int i = 0; i < 18; i++)
		s.syncAsSint16LE(_Inventory[i]);

	for (int i = 0; i < 99; i++)
		s.syncAsSint16LE(_Nodes[i]);

	s.syncAsByte(_OldEye);
	s.syncAsByte(_fstFwd);
	s.syncAsByte(_Help);
	s.syncAsByte(_ItemInhibit);
	s.syncAsByte(_LastObs);
	s.syncAsByte(_LastSound);
	s.syncAsByte(_MapEx);
	s.syncAsByte(_NoMusic);
	s.syncAsByte(_OldPouch);
	s.syncAsByte(_Replay);
	s.syncAsByte(_Spell1);
	s.syncAsByte(_Spell2);
	s.syncAsByte(_Spell3);
	s.syncAsByte(_TideCntl);
	s.syncAsByte(_Wizard);
	s.syncAsByte(_TSIconOnly);
	s.syncAsByte(_CTimerFlag);
	s.syncAsByte(_SkylarTimerFlag);

	// Present in the original. Looks unused.
	// s.syncAsSint16LE(_StatMap);
}

const char *const SAVEGAME_STR = "KTFR";
#define SAVEGAME_STR_SIZE 4
#define KTFR_SAVEGAME_VERSION 1

void KingdomGame::writeSavegameHeader(Common::OutSaveFile *out, KingdomSavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(KTFR_SAVEGAME_VERSION);

	// Write savegame name
	out->writeString(header._saveName);
	out->writeByte('\0');

	Common::MemoryWriteStreamDynamic *tempThumbnail = new Common::MemoryWriteStreamDynamic;
	Graphics::saveThumbnail(*tempThumbnail);
	out->write(tempThumbnail->getData(), tempThumbnail->size());
	delete tempThumbnail;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
}

bool KingdomGame::readSavegameHeader(Common::InSaveFile *in, KingdomSavegameHeader &header) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];
	header._thumbnail = nullptr;

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header._version = in->readByte();
	if (header._version > KTFR_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		header._saveName += ch;

	// Get the thumbnail
	header._thumbnail = Graphics::loadThumbnail(*in);
	if (!header._thumbnail)
		return false;

	// Read in save date/time
	header._year = in->readSint16LE();
	header._month = in->readSint16LE();
	header._day = in->readSint16LE();
	header._hour = in->readSint16LE();
	header._minute = in->readSint16LE();

	return true;
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
	debug("PlaySound %d : %s", idx, _rezNames[realIdx]);
	LoadAResource(realIdx);

	Common::SeekableReadStream *soundStream = _RezPointers[realIdx];
	Audio::RewindableAudioStream *rewindableStream = Audio::makeRawStream(soundStream, 22050, Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::NO);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, Audio::Mixer::kMaxMixerVolume);
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, rewindableStream);
//  In the original, there's an array describing whether a sound should loop or not.
//  The array is full of 'false'. If a variant uses looping sound/music, the following code
//	and the loop array should be added.
//	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(rewindableStream, _loopArray[idx]);
//	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, stream);
}

void KingdomGame::EraseCursor() {
	debug("STUB: EraseCursor");
}

void KingdomGame::ReadMouse() {
	_MouseButton = 0;

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
		_statPlay = 600;
		_loopFlag = true;
	}

	if (_UserInput == 0x42B && _statPlay == 53 && _gameMode == 0) {
		_oldStatPlay = _statPlay;
		_statPlay = 900;
		_loopFlag = true;
	}

	if (_UserInput == 0x12D && _CurrMap == 1)
		// CHECKME: _quitFlag = 2;
		_quit = true;
}

void KingdomGame::EraseCursorAsm() {
	debug("STUB: EraseCursorAsm");
}

void KingdomGame::DrawLocation() {
	if (_DaelonCntr > 0)
		_DaelonCntr--;

	PlaySound(0);
	_IconsClosed = true;
	_TSIconOnly = false;
	_ATimer = 0;
	_ATimerFlag = false;

	int emlValue = _emlTable[_NodeNum];
	if (emlValue > 0)
		EnAll();

	if (!_MapEx || !emlValue || _Resurrect) {
		if (_statPlay != 50)
			_Resurrect = false;
		_IconsClosed = false;
	} else {
		_MapEx = false;
		SaveAS();
		FShowPic(emlValue);
		_BTimer = 16;
		while(_BTimer) {
			checkTimers();
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
	int var6 = _zoomTable[mapNum][zoom][0];
	if (!_ASMode)
		SwitchAtoM();
	FShowPic(var6);
	_CurrMap = _zoomTable[mapNum][zoom][1];

	if (zoom > 0)
		_TreeLeftSta = _zoomTable[mapNum][zoom - 1][0] == 0 ? 0 : 3;
	else
		_TreeLeftSta = 0;

	if (zoom < 8)
		_TreeRightSta = _zoomTable[mapNum][zoom + 1][0] == 0 ? 0 : 2;
	else
		_TreeRightSta = 0;
}

void KingdomGame::ProcessMapInput(int mapNum) {
	switch(_UserInput) {
	case 0x43B:
	case 0x443:
		SwitchMtoA();
		_MapStat = 0;
		_statPlay--;
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
			_statPlay = _mapExit[_UserInput - 0x400];
			_MapEx = true;
			_loopFlag = true;
			SwitchAS();
		}

		if (_UserInput > 0x440) {
			SwitchMtoA();
			_MapStat = false;
			_statPlay--;
			_loopFlag = true;
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
		checkTimers();
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
	if (_Nodes[29] == 1 || _Nodes[68] == 1)
		return;

	if (_TSIconOnly != 0)
		wrkNodeNum = 79;

	if (_NodeNum == 56 && _Inventory[8] < 1 && _Wizard)
		wrkNodeNum = 80;
	
	for (int i = 0; i < 7; i++) {
		int idx = _iconActTable[wrkNodeNum][i];
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
//	No implementation needed in ScummVM
}

void KingdomGame::IncreaseHealth() {
	if (_Health <= 3)
		_Health = 4;
	else if (_Health <= 7)
		_Health = 8;
	else
		_Health = 12;
}

void KingdomGame::CheckMainScreen() {
	if (_CTimerFlag || _statPlay == 900 || _statPlay == 901)
		return;

	_CTimerFlag = false;
	if (_noIFScreen)
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
		if (_NodeNum == 21 && _Nodes[21] == 9)
			wrkNodeNum = 81;
		int idx = _iconActTable[wrkNodeNum][i];

		if (_Inventory[idx] >= 1 && _Nodes[29] != 1 && _Nodes[68] != 1 && !_ItemInhibit && !_IconsClosed) {
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
			_TreeEyePic = _teaSeq[_TreeEyeSta][0];
			DrawIcon(261, 51, _TreeEyePic);
			_TreeEyeTimer = _teaSeq[_TreeEyeSta][1];
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
		_TreeHGPic = _hgaSeq[_TreeHGSta][0];
		DrawIcon(249, 185, _TreeHGPic);
		_TreeHGTimer = _hgaSeq[_TreeHGSta][1];
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

	if (_Nodes[29] || _rnd->getRandomNumber(6) == 0) {
		if (!_Nodes[49] || _RobberyNode != _NodeNum) {
			if (_LastObstacle != _NodeNum) {
				if (_rnd->getRandomNumber(5) == 0) {
					_statPlay = 250;
					_LastObstacle = _NodeNum;
					_LastObs = true;
					_loopFlag = true;
					return true;
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else {
			_statPlay = 490;
			_loopFlag = true;
			return true;
		}
	} else {
		_statPlay = 280;
		_RobberyNode = _NodeNum;
		_LastObstacle = _NodeNum;
		_LastObs = true;
		_loopFlag = true;
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
	const byte *data = _kingartEntries[index]._data;
	int width = _kingartEntries[index]._width;
	int height = _kingartEntries[index]._height;

	::Graphics::Surface *screen = g_system->lockScreen();
	for (int curX = 0; curX < width; curX++) {
		for (int curY = 0; curY < height; curY++) {
			const byte *src = data + (curY * width) + curX;
			byte *dst = (byte *)screen->getBasePtr(curX + x, curY + y);
			if (*src != 0xFF)
				*dst = *src;
		}
	}
	g_system->unlockScreen();
	g_system->updateScreen();

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
			else if (event.kbd.keycode == Common::KEYCODE_c && event.kbd.hasFlags(Common::KBD_CTRL)) {
				_UserInput = 0x12D;
				// _QuitFlag = 2;
				_quit = true; 
			}
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
			} else if (_SkylarTimerFlag) {
				_SkylarTimerFlag = false;
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
	if (_CurrMap != 1 && _statPlay >= 30) {
		int var2 = _statPlay == 901 ? 16 : 0;
		int var6 = _statPlay == 901 ? 35 : 16;
		for (int i = 0; i < var6 + 1; i++) {
			if (i == var6) {
				int tmpVal = checkMouseMapAS();
				if (tmpVal == -1) {
					CursorTypeExit();
					return;
				} else
					_MouseValue = tmpVal;
			} else if (_CursorX >= _mouseMapMS[var2 + i]._minX && _CursorX < _mouseMapMS[var2 + i]._maxX && _CursorY >= _mouseMapMS[var2 + i]._minY && _CursorY < _mouseMapMS[var2 + i]._maxY) {
				_MouseValue = _mouseMapMS[var2 + i]._mouseValue;
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
			_MouseValue = !_ASMode ? 0x43A : 0x43B;
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
		// Restore game.
		// No more check in ScummVM, we display the load screen
		break;
	case 0x407:
		if (_statPlay == 182 && _Nodes[18] < 9)
			_MouseValue = 0;
		break;
	case 0x40D:
		if (_Nodes[29] == 1)
			_MouseValue = 0;
		break;
	case 0x41F:
		if (_Nodes[32] == 0)
			_MouseValue = 0;
		break;
	case 0x422:
	case 0x425:
		if (!_Wizard)
			_MouseValue = 0;
		break;
	case 0x428:
		if (_NodeNum == 5 && _gameMode != 2 && _Spell1)
			_MouseValue = 0;
		break;
	case 0x42A:
		if (_NodeNum == 5 && _gameMode != 2 && _Spell2)
			_MouseValue = 0;
		break;
	case 0x42B:
		if (_NodeNum == 5 && _gameMode != 2 && _Spell3)
			_MouseValue = 0;
		break;
	case 0x445:
		if (_statPlay == 161 && _Nodes[16] == 0 && _Wizard)
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
			int indx = _iconActTable[var2][var6];
			if (_Inventory[indx] != 0 && _Nodes[29] != 1 && _Nodes[68] != 1 && !_IconsClosed && !_ItemInhibit) {
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
		_CursorDef = _cursorTable[_MouseValue - 0x400];
	else 
		_CursorDef = (_MouseValue != 0) ? 0x68 : 0x67;
}

int KingdomGame::checkMouseMapAS() {
	for (int i = 0; i < 16; i++) {
		if (_CursorX >= _mouseMapAS[_CurrMap][i]._minX && _CursorX < _mouseMapAS[_CurrMap][i]._maxX
			&& _CursorY >= _mouseMapAS[_CurrMap][i]._minY && _CursorY < _mouseMapAS[_CurrMap][i]._maxY)
			return _mouseMapAS[_CurrMap][i]._mouseValue;
	}
	if (_CurrMap == 11) {
		for (int i = 0; i < 16; i++) {
			if (_CursorX >= _mouseMapAS[12][i]._minX && _CursorX < _mouseMapAS[12][i]._maxX
				&& _CursorY >= _mouseMapAS[12][i]._minY && _CursorY < _mouseMapAS[12][i]._maxY)
				return _mouseMapAS[12][i]._mouseValue;
		}
	}
	return -1;
}
void KingdomGame::SetCursor(int cursor) {
	KingArtEntry Cursor = _kingartEntries[cursor];
	CursorMan.replaceCursor(Cursor._data, Cursor._width, Cursor._height, 0, 0, 255);
}

} // End of namespace Kingdom
