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
 * $URL$
 * $Id$
 *
 */

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/resource.h"
#include "kyra/sound.h"
#include "kyra/util.h"

#include "common/endian.h"

namespace Kyra {

LoLEngine::LoLEngine(OSystem *system, const GameFlags &flags) : KyraEngine_v1(system, flags) {
	_screen = 0;
	
	switch (_flags.lang) {
	case Common::EN_ANY:
	case Common::EN_USA:
	case Common::EN_GRB:
		_lang = 0;
		break;

	case Common::FR_FRA:
		_lang = 1;
		break;

	case Common::DE_DEU:
		_lang = 2;
		break;

	default:
		warning("unsupported language, switching back to English");
		_lang = 0;
		break;
	}

	memset(_shapes, 0, sizeof(_shapes));
	
	_chargenWSA = 0;
	_lastUsedStringBuffer = 0;
	_landsFile = 0;
}

LoLEngine::~LoLEngine() {
	setupPrologueData(false);

	for (uint i = 0; i < ARRAYSIZE(_shapes); ++i)
		delete[] _shapes[i];
	delete[] _landsFile;

	delete _screen;
	delete _tim;
	
	for (Common::Array<const TIMOpcode*>::iterator i = _timIntroOpcodes.begin(); i != _timIntroOpcodes.end(); ++i)
		delete *i;
	_timIntroOpcodes.clear();
}

Screen *LoLEngine::screen() {
	return _screen;
}

Common::Error LoLEngine::init() {
	_screen = new Screen_LoL(this, _system);
	assert(_screen);
	_screen->setResolution();

	KyraEngine_v1::init();
	
	_tim = new TIMInterpreter(this, _screen, _system);
	assert(_tim);
	
	_screen->setAnimBlockPtr(10000);
	_screen->setScreenDim(0);

	if (!_sound->init())
		error("Couldn't init sound");

	return Common::kNoError;
}

Common::Error LoLEngine::go() {
	bool hasSave = saveFileLoadable(0);

	if (!hasSave) {
		setupPrologueData(true);
		showIntro();
		setupPrologueData(false);
	}

	preInit();

	int processSelection = -1;
	while (!shouldQuit() && processSelection == -1) {
		_screen->loadBitmap("TITLE.CPS", 2, 2, _screen->getPalette(0));
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->fadePalette(_screen->getPalette(0), 0x1E);

		int selection = mainMenu();

		switch (selection) {
		case 0:		// New game
			processSelection = 0;
			break;

		case 1:		// Show intro
			setupPrologueData(true);
			_screen->hideMouse();
			showIntro();
			_screen->showMouse();
			setupPrologueData(false);
			break;

		case 2:		// "Lore of the Lands"
			break;

		case 3:		// Load game
			// For now fall through
			//processSelection = 3;
			//break;

		case 4:		// Quit game
		default:
			quitGame();
			break;
		}
	}

	if (processSelection == -1)
		return Common::kNoError;

	if (processSelection == 0) {
		// Unlike the original, we add a nice fade to black
		memset(_screen->getPalette(0), 0, 768);
		_screen->fadePalette(_screen->getPalette(0), 0x54);

		setupPrologueData(true);
		_sound->loadSoundFile("LOREINTR");
		_sound->playTrack(6);
		/*int character = */chooseCharacter();
		_sound->playTrack(1);
		_screen->fadeToBlack();
		setupPrologueData(false);
	} else if (processSelection == 3) {
		//XXX
	}

	return Common::kNoError;
}

#pragma mark - Initialization

void LoLEngine::preInit() {
	debugC(9, kDebugLevelMain, "LoLEngine::preInit()");

	_res->loadFileList("FILEDATA.FDT");
	_screen->loadFont(Screen::FID_9_FNT, "FONT9P.FNT");
	_screen->loadFont(Screen::FID_6_FNT, "FONT6P.FNT");

	uint8 *pal = _screen->getPalette(0);
	memset(pal, 0, 768);
	_screen->setScreenPalette(pal);

	/*if (_sound->getMusicType() == Sound::kMidiMT32 || _sound->getSfxType() == Sound::kMidiMT32) {
		_sound->loadSoundFile("LOLSYSEX");
		_sound->playTrack(0);

		while (_sound->isPlaying() && !shouldQuit())
			delay(10);
	}*/

	if (shouldQuit())
		return;

	_eventList.clear();

	//loadTalkFile(0);
	
	char filename[32];
	snprintf(filename, sizeof(filename), "LANDS.%s", _languageExt[_lang]);	
	_landsFile = _res->fileData(filename, 0);

	initializeCursors();

	/*_screen->setFont(Screen::FID_6_FNT);
	_screen->fprintString("V CD1.02 D", 260, 301, 0x67, 0x00, 0x04);*/
	_screen->setFont(Screen::FID_9_FNT);
}

void LoLEngine::initializeCursors() {
	debugC(9, kDebugLevelMain, "LoLEngine::initializeCursors()");

	_screen->loadBitmap("ITEMICN.SHP", 3, 3, 0);
	_shapes[0] = _screen->makeShapeCopy(_screen->getCPagePtr(3), 0);
	_screen->setMouseCursor(0, 0, _shapes[0]);
}


int LoLEngine::mainMenu() {
	debugC(9, kDebugLevelMain, "LoLEngine::mainMenu()");

	bool hasSave = saveFileLoadable(0);

	MainMenu::StaticData data = {
		{ 0, 0, 0, 0, 0 }, 
		{ 0x01, 0x04, 0x0C, 0x04, 0x00, 0x3D, 0x9F },
		{ 0x2C, 0x19, 0x48, 0x2C },
		Screen::FID_9_FNT, 1
	};

	if (hasSave)
		++data.menuTable[3];

	static const uint16 mainMenuStrings[2][5] = {
		{ 0x4248, 0x4249, 0x42DD, 0x424A, 0x0000 },
		{ 0x4248, 0x4249, 0x42DD, 0x4001, 0x424A }
	};

	for (int i = 0; i < 5; ++i) {
		if (hasSave)
			data.strings[i] = getLangString(mainMenuStrings[1][i]);		
		else
			data.strings[i] = getLangString(mainMenuStrings[0][i]);		
	}

	MainMenu *menu = new MainMenu(this);
	assert(menu);
	menu->init(data, MainMenu::Animation());

	int selection = menu->handle(hasSave ? 12 : 6);
	delete menu;

	if (!hasSave && selection == 3)
		selection = 4;

	return selection;
}

#pragma mark - Localization

const char *LoLEngine::getLangString(uint16 id) {
	debugC(9, kDebugLevelMain, "LoLEngine::getLangString(0x%.04X)", id);

	if (id == 0xFFFF)
		return 0;

	uint16 realId = id & 0x3FFF;
	uint8 *buffer = 0;

	if (id & 0x4000)
		buffer = _landsFile;
	else
		buffer = 0;	// TODO

	if (!buffer)
		return 0;

	const char *string = (const char *)getTableEntry(buffer, realId);

	char *srcBuffer = _stringBuffer[_lastUsedStringBuffer];
	Util::decodeString1(string, srcBuffer);
	Util::decodeString2(srcBuffer, srcBuffer);

	++_lastUsedStringBuffer;
	_lastUsedStringBuffer %= ARRAYSIZE(_stringBuffer);

	return srcBuffer;
}

uint8 *LoLEngine::getTableEntry(uint8 *buffer, uint16 id) {
	debugC(9, kDebugLevelMain, "LoLEngine::getTableEntry(%p, %d)", (const void *)buffer, id);
	if (!buffer)
		return 0;

	return buffer + READ_LE_UINT16(buffer + (id<<1));
}

#pragma mark - Intro

void LoLEngine::setupPrologueData(bool load) {
	debugC(9, kDebugLevelMain, "LoLEngine::setupPrologueData(%d)", load);

	static const char * const fileList[] = {
		"GENERAL.PAK", "INTROVOC.PAK", "STARTUP.PAK", "INTRO1.PAK",
		"INTRO2.PAK", "INTRO3.PAK", "INTRO4.PAK", "INTRO5.PAK",
		"INTRO6.PAK", "INTRO7.PAK", "INTRO8.PAK", "INTRO9.PAK"
	};

	char filename[32];
	for (uint i = 0; i < ARRAYSIZE(fileList); ++i) {
		filename[0] = '\0';

		if (_flags.isTalkie) {
			strcpy(filename, _languageExt[_lang]);
			strcat(filename, "/");
		}

		strcat(filename, fileList[i]);
		
		if (load) {
			if (!_res->loadPakFile(filename))
				error("Couldn't load file: '%s'", filename);
		} else {
			_res->unloadPakFile(filename);
		}
	}

	_screen->clearPage(0);
	_screen->clearPage(3);
	
	if (load) {
		_chargenWSA = new WSAMovie_v2(this, _screen);
		assert(_chargenWSA);

		_charSelection = -1;
		_charSelectionInfoResult = -1;

		_selectionAnimFrames[0] = _selectionAnimFrames[2] = 0;
		_selectionAnimFrames[1] = _selectionAnimFrames[3] = 1;

		memset(_selectionAnimTimers, 0, sizeof(_selectionAnimTimers));
		memset(_screen->getPalette(1), 0, 768);
	} else {
		delete _chargenWSA; _chargenWSA = 0;
	}
}

void LoLEngine::showIntro() {
	debugC(9, kDebugLevelMain, "LoLEngine::showIntro()");

	TIM *intro = _tim->load("LOLINTRO.TIM", &_timIntroOpcodes);

	_screen->loadFont(Screen::FID_8_FNT, "NEW8P.FNT");
	_screen->loadFont(Screen::FID_INTRO_FNT, "INTRO.FNT");
	_screen->setFont(Screen::FID_8_FNT);

	_tim->resetFinishedFlag();
	_tim->setLangData("LOLINTRO.DIP");

	_screen->hideMouse();

	uint32 palNextFadeStep = 0;
	while (!_tim->finished() && !shouldQuit() && !skipFlag()) {
		updateInput();
		_tim->exec(intro, false);
		_screen->checkedPageUpdate(8, 4);

		if (_tim->_palDiff) {
			if (palNextFadeStep < _system->getMillis()) {
				_tim->_palDelayAcc += _tim->_palDelayInc;
				palNextFadeStep = _system->getMillis() + ((_tim->_palDelayAcc >> 8) * _tickLength);
				_tim->_palDelayAcc &= 0xFF;

				if (!_screen->fadePalStep(_screen->getPalette(0), _tim->_palDiff)) {
					_screen->setScreenPalette(_screen->getPalette(0));
					_tim->_palDiff = 0;
				}
			}
		}

		_system->delayMillis(10);
		_screen->updateScreen();
	}
	_screen->showMouse();
	_sound->voiceStop();
	
	_eventList.clear();
	
	_tim->unload(intro);
	_tim->clearLangData();
	
	_screen->fadePalette(_screen->getPalette(1), 30, 0);
}

int LoLEngine::chooseCharacter() {
	debugC(9, kDebugLevelMain, "LoLEngine::chooseCharacter()");

	_tim->setLangData("LOLINTRO.DIP");
	
	_screen->loadFont(Screen::FID_9_FNT, "FONT9P.FNT");

	_screen->loadBitmap("ITEMICN.SHP", 3, 3, 0);
	_screen->setMouseCursor(0, 0, _screen->getPtrToShape(_screen->getCPagePtr(3), 0));

	while (!_screen->isMouseVisible())
		_screen->showMouse();

	_screen->loadBitmap("CHAR.CPS", 2, 2, _screen->getPalette(0));
	_screen->loadBitmap("BACKGRND.CPS", 4, 4, _screen->getPalette(0));
	
	if (!_chargenWSA->open("CHARGEN.WSA", 1, 0))
		error("Couldn't load CHARGEN.WSA");
	_chargenWSA->setX(113);
	_chargenWSA->setY(0);
	_chargenWSA->setDrawPage(2);
	_chargenWSA->displayFrame(0, 0, 0, 0);

	_screen->setFont(Screen::FID_9_FNT);
	_screen->_curPage = 2;
	
	for (int i = 0; i < 4; ++i)
		_screen->fprintStringIntro(_charPreviews[i].name, _charPreviews[i].x + 16, _charPreviews[i].y + 36, 0xC0, 0x00, 0x9C, 0x120);

	for (int i = 0; i < 4; ++i) {
		_screen->fprintStringIntro("%d", _charPreviews[i].x + 21, _charPreviews[i].y + 48, 0x98, 0x00, 0x9C, 0x220, _charPreviews[i].attrib[0]);
		_screen->fprintStringIntro("%d", _charPreviews[i].x + 21, _charPreviews[i].y + 56, 0x98, 0x00, 0x9C, 0x220, _charPreviews[i].attrib[1]);
		_screen->fprintStringIntro("%d", _charPreviews[i].x + 21, _charPreviews[i].y + 64, 0x98, 0x00, 0x9C, 0x220, _charPreviews[i].attrib[2]);
	}
	
	_screen->fprintStringIntro(_tim->getCTableEntry(51), 36, 173, 0x98, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(53), 36, 181, 0x98, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(55), 36, 189, 0x98, 0x00, 0x9C, 0x20);
	
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->_curPage = 0;
	
	_screen->fadePalette(_screen->getPalette(0), 30, 0);
	
	bool kingIntro = true;
	while (!shouldQuit()) {
		if (kingIntro)
			kingSelectionIntro();

		if (_charSelection < 0)
			processCharacterSelection();

		if (shouldQuit())
			break;

		if (_charSelection == 100) {
			kingIntro = true;
			_charSelection = -1;
			continue;
		}

		_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_screen->showMouse();

		if (selectionCharInfo(_charSelection) == -1) {
			_charSelection = -1;
			kingIntro = false;
		} else {
			break;
		}
	}

	if (shouldQuit())
		return -1;

	uint32 waitTime = _system->getMillis() + 420 * _tickLength;
	while (waitTime > _system->getMillis() && !skipFlag() && !shouldQuit()) {
		updateInput();
		_system->delayMillis(10);
	}

	// HACK: Remove all input events
	_eventList.clear();

	_tim->clearLangData();

	return _charSelection;
}

void LoLEngine::kingSelectionIntro() {
	debugC(9, kDebugLevelMain, "LoLEngine::kingSelectionIntro()");
	
	_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);
	int y = 38;
	
	_screen->fprintStringIntro(_tim->getCTableEntry(57), 8, y, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(58), 8, y + 10, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(59), 8, y + 20, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(60), 8, y + 30, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(61), 8, y + 40, 0x32, 0x00, 0x9C, 0x20);

	_sound->voicePlay("KING01");
	
	_chargenWSA->setX(113);
	_chargenWSA->setY(0);
	_chargenWSA->setDrawPage(0);
	
	int index = 4;
	while (_sound->voiceIsPlaying("KING01") && _charSelection == -1 && !shouldQuit() && !skipFlag()) {
		index = MAX(index, 4);

		_chargenWSA->displayFrame(_chargenFrameTable[index], 0, 0, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar1IdxTable[index]*2+0], _selectionPosTable[_selectionChar1IdxTable[index]*2+1], _charPreviews[0].x, _charPreviews[0].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar2IdxTable[index]*2+0], _selectionPosTable[_selectionChar2IdxTable[index]*2+1], _charPreviews[1].x, _charPreviews[1].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar3IdxTable[index]*2+0], _selectionPosTable[_selectionChar3IdxTable[index]*2+1], _charPreviews[2].x, _charPreviews[2].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar4IdxTable[index]*2+0], _selectionPosTable[_selectionChar4IdxTable[index]*2+1], _charPreviews[3].x, _charPreviews[3].y, 32, 32, 4, 0);
		_screen->updateScreen();

		uint32 waitEnd = _system->getMillis() + 7 * _tickLength;
		while (waitEnd > _system->getMillis() && _charSelection == -1 && !shouldQuit() && !skipFlag()) {
			_charSelection = getCharSelection();
			_system->delayMillis(10);
		}

		index = (index + 1) % 22;
	}
	
	resetSkipFlag();
	
	_chargenWSA->displayFrame(0x10, 0, 0, 0);
	_screen->updateScreen();
	_sound->voiceStop("KING01");
}

void LoLEngine::kingSelectionReminder() {
	debugC(9, kDebugLevelMain, "LoLEngine::kingSelectionReminder()");
	
	_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);
	int y = 48;
	
	_screen->fprintStringIntro(_tim->getCTableEntry(62), 8, y, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(63), 8, y + 10, 0x32, 0x00, 0x9C, 0x20);
	
	_sound->voicePlay("KING02");
	
	_chargenWSA->setX(113);
	_chargenWSA->setY(0);
	_chargenWSA->setDrawPage(0);
	
	int index = 0;
	while (_sound->voiceIsPlaying("KING02") && _charSelection == -1 && !shouldQuit() && index < 15) {
		_chargenWSA->displayFrame(_chargenFrameTable[index+9], 0, 0, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar1IdxTable[index]*2+0], _selectionPosTable[_reminderChar1IdxTable[index]*2+1], _charPreviews[0].x, _charPreviews[0].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar2IdxTable[index]*2+0], _selectionPosTable[_reminderChar2IdxTable[index]*2+1], _charPreviews[1].x, _charPreviews[1].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar3IdxTable[index]*2+0], _selectionPosTable[_reminderChar3IdxTable[index]*2+1], _charPreviews[2].x, _charPreviews[2].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar4IdxTable[index]*2+0], _selectionPosTable[_reminderChar4IdxTable[index]*2+1], _charPreviews[3].x, _charPreviews[3].y, 32, 32, 4, 0);
		_screen->updateScreen();

		uint32 waitEnd = _system->getMillis() + 8 * _tickLength;
		while (waitEnd > _system->getMillis() && !shouldQuit()) {
			_charSelection = getCharSelection();
			_system->delayMillis(10);
		}

		index = (index + 1) % 22;
	}

	_sound->voiceStop("KING02");
}

void LoLEngine::kingSelectionOutro() {
	debugC(9, kDebugLevelMain, "LoLEngine::kingSelectionOutro()");

	_sound->voicePlay("KING03");

	_chargenWSA->setX(113);
	_chargenWSA->setY(0);
	_chargenWSA->setDrawPage(0);

	int index = 0;
	while (_sound->voiceIsPlaying("KING03") && !shouldQuit() && !skipFlag()) {
		index = MAX(index, 4);

		_chargenWSA->displayFrame(_chargenFrameTable[index], 0, 0, 0);
		_screen->updateScreen();

		uint32 waitEnd = _system->getMillis() + 8 * _tickLength;
		while (waitEnd > _system->getMillis() && !shouldQuit() && !skipFlag()) {
			updateInput();
			_system->delayMillis(10);
		}

		index = (index + 1) % 22;
	}

	resetSkipFlag();

	_chargenWSA->displayFrame(0x10, 0, 0, 0);
	_screen->updateScreen();
	_sound->voiceStop("KING03");
}

void LoLEngine::processCharacterSelection() {
	debugC(9, kDebugLevelMain, "LoLEngine::processCharacterSelection()");
	
	_charSelection = -1;
	while (!shouldQuit() && _charSelection == -1) {
		uint32 nextKingMessage = _system->getMillis() + 900 * _tickLength;

		while (nextKingMessage > _system->getMillis() && _charSelection == -1 && !shouldQuit()) {
			updateSelectionAnims();
			_charSelection = getCharSelection();
			_system->delayMillis(10);
		}

		if (_charSelection == -1)
			kingSelectionReminder();
	}
}

void LoLEngine::updateSelectionAnims() {
	debugC(9, kDebugLevelMain, "LoLEngine::updateSelectionAnims()");

	for (int i = 0; i < 4; ++i) {
		if (_system->getMillis() < _selectionAnimTimers[i])
			continue;

		const int index = _selectionAnimIndexTable[_selectionAnimFrames[i] + i * 2];
		_screen->copyRegion(_selectionPosTable[index*2+0], _selectionPosTable[index*2+1], _charPreviews[i].x, _charPreviews[i].y, 32, 32, 4, 0);

		int delayTime = 0;
		if (_selectionAnimFrames[i] == 1)
			delayTime = _rnd.getRandomNumberRng(0, 31) + 80;
		else
			delayTime = _rnd.getRandomNumberRng(0, 3) + 10;

		_selectionAnimTimers[i] = _system->getMillis() + delayTime * _tickLength;
		_selectionAnimFrames[i] = (_selectionAnimFrames[i] + 1) % 2;
	}

	_screen->updateScreen();
}

int LoLEngine::selectionCharInfo(int character) {
	debugC(9, kDebugLevelMain, "LoLEngine::selectionCharInfo(%d)", character);
	if (character < 0)
		return -1;

	char filename[16];
	char vocFilename[6];
	strcpy(vocFilename, "000X0");

	switch (character) {
	case 0:
		strcpy(filename, "FACE09.SHP");
		vocFilename[3] = 'A';
		break;
	
	case 1:
		strcpy(filename, "FACE01.SHP");
		vocFilename[3] = 'M';
		break;
	
	case 2:
		strcpy(filename, "FACE08.SHP");
		vocFilename[3] = 'K';
		break;
	
	case 3:
		strcpy(filename, "FACE05.SHP");
		vocFilename[3] = 'C';
		break;
	
	default:
		break;
	};

	_screen->loadBitmap(filename, 9, 9, 0);
	_screen->copyRegion(0, 122, 0, 122, 320, 78, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(_charPreviews[character].x - 3, _charPreviews[character].y - 3, 8, 127, 38, 38, 2, 0);

	static const uint8 charSelectInfoIdx[] = { 0x1D, 0x22, 0x27, 0x2C };
	const int idx = charSelectInfoIdx[character];

	_screen->fprintStringIntro(_tim->getCTableEntry(idx+0), 50, 127, 0x53, 0x00, 0xCF, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(idx+1), 50, 137, 0x53, 0x00, 0xCF, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(idx+2), 50, 147, 0x53, 0x00, 0xCF, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(idx+3), 50, 157, 0x53, 0x00, 0xCF, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(idx+4), 50, 167, 0x53, 0x00, 0xCF, 0x20);
	
	_screen->fprintStringIntro(_tim->getCTableEntry(69), 100, 168, 0x32, 0x00, 0xCF, 0x20);
	
	selectionCharInfoIntro(vocFilename);
	if (_charSelectionInfoResult == -1) {
		while (_charSelectionInfoResult == -1) {
			_charSelectionInfoResult = selectionCharAccept();
			_system->delayMillis(10);
		}
	}
	
	if (_charSelectionInfoResult != 1) {
		_charSelectionInfoResult = -1;
		_screen->copyRegion(0, 122, 0, 122, 320, 78, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		return -1;
	}

	_screen->copyRegion(48, 127, 48, 127, 272, 60, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->hideMouse();
	_screen->copyRegion(48, 127, 48, 160, 272, 35, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);

	_screen->fprintStringIntro(_tim->getCTableEntry(64), 3, 28, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(65), 3, 38, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(66), 3, 48, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(67), 3, 58, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(68), 3, 68, 0x32, 0x00, 0x9C, 0x20);

	resetSkipFlag();
	kingSelectionOutro();	
	return character;
}

void LoLEngine::selectionCharInfoIntro(char *file) {
	debugC(9, kDebugLevelMain, "LoLEngine::selectionCharInfoIntro(%p)", (const void *)file);
	int index = 0;
	file[4] = '0';
	
	while (_charSelectionInfoResult == -1 && !shouldQuit()) {
		if (!_sound->voicePlay(file))
			break;

		int i = 0;
		while (_sound->voiceIsPlaying(file) && _charSelectionInfoResult == -1 && !shouldQuit()) {
			_screen->drawShape(0, _screen->getPtrToShape(_screen->getCPagePtr(9), _charInfoFrameTable[i]), 11, 130, 0, 0);
			_screen->updateScreen();

			uint32 nextFrame = _system->getMillis() + 8 * _tickLength;
			while (nextFrame > _system->getMillis() && _charSelectionInfoResult == -1) {
				_charSelectionInfoResult = selectionCharAccept();
				_system->delayMillis(10);
			}

			i = (i + 1) % 32;
		}

		_sound->voiceStop(file);
		file[4] = ++index + '0';
	}

	_screen->drawShape(0, _screen->getPtrToShape(_screen->getCPagePtr(9), 0), 11, 130, 0, 0);
	_screen->updateScreen();
}

int LoLEngine::getCharSelection() {
	int inputFlag = checkInput(0, false) & 0xCF;
	removeInputTop();

	if (inputFlag == 200) {
		for (int i = 0; i < 4; ++i) {
			if (_charPreviews[i].x <= _mouseX && _mouseX <= _charPreviews[i].x + 31 &&
				_charPreviews[i].y <= _mouseY && _mouseY <= _charPreviews[i].y + 31)
				return i;
		}
	}
	
	return -1;
}

int LoLEngine::selectionCharAccept() {
	int inputFlag = checkInput(0, false) & 0xCF;
	removeInputTop();
	
	if (inputFlag == 200) {
		if (88 <= _mouseX && _mouseX <= 128 && 180 <= _mouseY && _mouseY <= 194)
			return 1;
		if (196 <= _mouseX && _mouseX <= 236 && 180 <= _mouseY && _mouseY <= 194)
			return 0;
	}
	
	return -1;
}

#pragma mark - Opcodes

typedef Common::Functor2Mem<const TIM *, const uint16 *, int, LoLEngine> TIMOpcodeLoL;
#define SetTimOpcodeTable(x) timTable = &x;
#define OpcodeTim(x) timTable->push_back(new TIMOpcodeLoL(this, &LoLEngine::x))
#define OpcodeTimUnImpl() timTable->push_back(new TIMOpcodeLoL(this, 0))

void LoLEngine::setupOpcodeTable() {
	Common::Array<const TIMOpcode*> *timTable = 0;

	SetTimOpcodeTable(_timIntroOpcodes);
	
	// 0x00
	OpcodeTim(tlol_setupPaletteFade);
	OpcodeTimUnImpl();
	OpcodeTim(tlol_loadPalette);
	OpcodeTim(tlol_setupPaletteFadeEx);
	
	// 0x04
	OpcodeTim(tlol_processWsaFrame);
	OpcodeTim(tlol_displayText);
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();
}

#pragma mark -

int LoLEngine::tlol_setupPaletteFade(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::t2_playSoundEffect(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	_screen->getFadeParams(_screen->getPalette(0), param[0], _tim->_palDelayInc, _tim->_palDiff);
	_tim->_palDelayAcc = 0;
	return 1;
}

int LoLEngine::tlol_loadPalette(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_loadPalette(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	const char *palFile = (const char *)(tim->text + READ_LE_UINT16(tim->text + (param[0]<<1)));
	_res->loadFileToBuf(palFile, _screen->getPalette(0), 768);
	return 1;
}

int LoLEngine::tlol_setupPaletteFadeEx(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_setupPaletteFadeEx(%p, %p) (%d)", (const void*)tim, (const void*)param, param[0]);
	memcpy(_screen->getPalette(0), _screen->getPalette(1), 768);

	_screen->getFadeParams(_screen->getPalette(0), param[0], _tim->_palDelayInc, _tim->_palDiff);
	_tim->_palDelayAcc = 0;
	return 1;
}

int LoLEngine::tlol_processWsaFrame(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_processWsaFrame(%p, %p) (%d, %d, %d, %d, %d)",
		(const void*)tim, (const void*)param, param[0], param[1], param[2], param[3], param[4]);
	TIMInterpreter::Animation *anim = (TIMInterpreter::Animation *)tim->wsa[param[0]].anim;
	const int frame = param[1];
	const int x2 = param[2];
	const int y2 = param[3];
	const int factor = MAX<int>(0, (int16)param[4]);
	
	const int x1 = anim->x;
	const int y1 = anim->y;
	
	int w1 = anim->wsa->width();
	int h1 = anim->wsa->height();
	int w2 = (w1 * factor) / 100;
	int h2 = (h1 * factor) / 100;
	
	anim->wsa->setDrawPage(2);
	anim->wsa->setX(x1);
	anim->wsa->setY(y1);
	anim->wsa->displayFrame(frame, anim->wsaCopyParams & 0xF0FF, 0, 0);
	_screen->wsaFrameAnimationStep(x1, y1, x2, y2, w1, h1, w2, h2, 2, 8, 0);
	_screen->checkedPageUpdate(8, 4);
	_screen->updateScreen();

	return 1;
}

int LoLEngine::tlol_displayText(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_displayText(%p, %p) (%d, %d)", (const void*)tim, (const void*)param, param[0], (int16)param[1]);
	_tim->displayText(param[0], param[1]);
	return 1;
}

} // end of namespace Kyra

