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

#ifdef ENABLE_LOL

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/resource.h"

#include "base/version.h"

namespace Kyra {

#pragma mark - Intro

int LoLEngine::processPrologue() {
	debugC(9, kDebugLevelMain, "LoLEngine::processPrologue()");

	setupPrologueData(true);

	if (!saveFileLoadable(0) || _flags.isDemo)
		showIntro();

	if (_flags.isDemo)
		return -1;

	preInit();

	int processSelection = -1;
	while (!shouldQuit() && processSelection == -1) {
		_screen->loadBitmap("TITLE.CPS", 2, 2, _screen->getPalette(0));
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);

		_screen->setFont(Screen::FID_6_FNT);
		// Original version: (260|193) "V CD1.02 D"
		const int width = _screen->getTextWidth(gScummVMVersion);
		_screen->fprintString("SVM %s", 300 - width, 193, 0x67, 0x00, 0x04, gScummVMVersion);
		_screen->setFont(Screen::FID_9_FNT);

		_screen->fadePalette(_screen->getPalette(0), 0x1E);
		_screen->updateScreen();

		_eventList.clear();
		int selection = mainMenu();
		_screen->hideMouse();

		// Unlike the original, we add a nice fade to black
		memset(_screen->getPalette(0), 0, 768);
		_screen->fadePalette(_screen->getPalette(0), 0x54);

		switch (selection) {
		case 0:		// New game
			processSelection = 0;
			break;

		case 1:		// Show intro
			showIntro();
			break;

		case 2:		// "Lore of the Lands" (only CD version)
			break;

		case 3:		// Load game
			// For now fall through
			//processSelection = 3;
			//break;

		case 4:		// Quit game
		default:
			quitGame();
			updateInput();
			break;
		}
	}

	if (processSelection == 0 || processSelection == 3) {
		_sound->loadSoundFile(0);
		_sound->playTrack(6);
		chooseCharacter();
		_sound->playTrack(1);
		_screen->fadeToBlack();
	}

	setupPrologueData(false);

	return processSelection;
}

void LoLEngine::setupPrologueData(bool load) {
	debugC(9, kDebugLevelMain, "LoLEngine::setupPrologueData(%d)", load);

	static const char * const fileListCD[] = {
		"GENERAL.PAK", "INTROVOC.PAK", "STARTUP.PAK", "INTRO1.PAK",
		"INTRO2.PAK", "INTRO3.PAK", "INTRO4.PAK", "INTRO5.PAK",
		"INTRO6.PAK", "INTRO7.PAK", "INTRO8.PAK", "INTRO9.PAK", 0
	};

	static const char * const fileListFloppyExtracted[] = {
		"INTRO.PAK", "INTROVOC.PAK", 0
	};

	static const char * const fileListFloppy[] = {
		"INTRO.PAK", "INTROVOC.CMP", 0
	};

	const char * const *fileList = _flags.isTalkie ? fileListCD :
		(_flags.useInstallerPackage ? fileListFloppy : fileListFloppyExtracted);


	char filename[32];
	for (uint i = 0; fileList[i]; ++i) {
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

		//_charSelection = -1;
		_charSelectionInfoResult = -1;

		_selectionAnimFrames[0] = _selectionAnimFrames[2] = 0;
		_selectionAnimFrames[1] = _selectionAnimFrames[3] = 1;

		memset(_selectionAnimTimers, 0, sizeof(_selectionAnimTimers));
		memset(_screen->getPalette(1), 0, 768);

		_sound->setSoundList(&_soundData[kMusicIntro]);
	
		// We have three sound.dat files, one for the intro, one for the
		// end sequence and one for ingame, each contained in a different
		// PAK file. Therefore a new call to loadSoundFile() is required
		// whenever the PAK file configuration changes.
		if (_flags.platform == Common::kPlatformPC98)
			_sound->loadSoundFile("sound.dat");
	} else {
		delete _chargenWSA; _chargenWSA = 0;
		
		uint8 *pal = _screen->getPalette(0);
		memset(pal, 0, 768);
		_screen->setScreenPalette(pal);

		if (shouldQuit())
			return;

		_eventList.clear();
		_sound->setSoundList(0);
	}
}

void LoLEngine::showIntro() {
	debugC(9, kDebugLevelMain, "LoLEngine::showIntro()");

	_tim = new TIMInterpreter(this, _screen, _system);
	assert(_tim);

	if (_flags.platform == Common::kPlatformPC98)
		showStarcraftLogo();

	uint8 *pal = _screen->getPalette(0);
	memset(pal, 0, 768);
	_screen->setScreenPalette(pal);

	_screen->clearPage(0);
	_screen->clearPage(4);
	_screen->clearPage(8);

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
	_sound->beginFadeOut();

	_eventList.clear();

	_tim->unload(intro);
	_tim->clearLangData();

	for (int i = 0; i < TIM::kWSASlots; i++)
		_tim->freeAnimStruct(i);

	delete _tim;
	_tim = 0;

	_screen->fadePalette(_screen->getPalette(1), 30, 0);
}

int LoLEngine::chooseCharacter() {
	debugC(9, kDebugLevelMain, "LoLEngine::chooseCharacter()");

	_tim = new TIMInterpreter(this, _screen, _system);
	assert(_tim);

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

	_chargenWSA->displayFrame(0, 2, 113, 0, 0, 0, 0);

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

	if (_flags.use16ColorMode)
		_screen->loadPalette("LOL.NOL", _screen->getPalette(0));

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

	delete _tim;
	_tim = 0;

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

	int index = 4;
	while ((!_speechFlag || (_speechFlag && _sound->voiceIsPlaying("KING01"))) && _charSelection == -1 && !shouldQuit() && !skipFlag()) {
		index = MAX(index, 4);

		_chargenWSA->displayFrame(_chargenFrameTable[index], 0, 113, 0, 0, 0, 0);
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

		if (_speechFlag)
			index = (index + 1) % 22;
		else if (++index >= 27)
			break;
	}

	resetSkipFlag();

	_chargenWSA->displayFrame(0x10, 0,113, 0, 0, 0, 0);
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

	int index = 0;
	while ((!_speechFlag || (_speechFlag && _sound->voiceIsPlaying("KING02"))) && _charSelection == -1 && !shouldQuit() && index < 15) {
		_chargenWSA->displayFrame(_chargenFrameTable[index+9], 0, 113, 0, 0, 0, 0);
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

		if (_speechFlag)
			index = (index + 1) % 22;
		else if (++index >= 27)
			break;
	}

	_sound->voiceStop("KING02");
}

void LoLEngine::kingSelectionOutro() {
	debugC(9, kDebugLevelMain, "LoLEngine::kingSelectionOutro()");

	_sound->voicePlay("KING03");

	int index = 0;
	while ((!_speechFlag || (_speechFlag && _sound->voiceIsPlaying("KING03"))) && !shouldQuit() && !skipFlag()) {
		index = MAX(index, 4);

		_chargenWSA->displayFrame(_chargenFrameTable[index], 0, 113, 0, 0, 0, 0);
		_screen->updateScreen();

		uint32 waitEnd = _system->getMillis() + 8 * _tickLength;
		while (waitEnd > _system->getMillis() && !shouldQuit() && !skipFlag()) {
			updateInput();
			_system->delayMillis(10);
		}

		if (_speechFlag)
			index = (index + 1) % 22;
		else if (++index >= 27)
			break;
	}

	resetSkipFlag();

	_chargenWSA->displayFrame(0x10, 0, 113, 0, 0, 0, 0);
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
	}

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

void LoLEngine::showStarcraftLogo() {
	WSAMovie_v2 *ci = new WSAMovie_v2(this, _screen);
	assert(ci);

	_screen->clearPage(0);
	_screen->clearPage(2);

	int endframe = ci->open("ci01.wsa", 0, _screen->_currentPalette);
	if (!ci->opened()) {
		delete ci;
		return;
	}
	_screen->hideMouse();
	ci->displayFrame(0, 2, 32, 80, 0);
	_screen->copyPage(2, 0);
	_screen->fadeFromBlack();
	int inputFlag = 0;
	for (int i = 0; i < endframe; i++) {
		inputFlag = checkInput(0) & 0xff;
		if (shouldQuit() || inputFlag)
			break;
		ci->displayFrame(i, 2, 32, 80, 0);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		delay(4 * _tickLength);
	}

	if (!(shouldQuit() || inputFlag)) {
		_sound->voicePlay("star2");
		while(_sound->voiceIsPlaying("star2") && !(shouldQuit() || inputFlag)) {
			inputFlag = checkInput(0) & 0xff;
			delay(_tickLength);			
		}
	}

	_screen->fadeToBlack();
	_screen->showMouse();

	_eventList.clear();
	delete ci;
}

} // end of namespace Kyra

#endif // ENABLE_LOL

