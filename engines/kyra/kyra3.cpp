/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "kyra/kyra.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"
#include "kyra/text.h"
#include "kyra/vqa.h"

#include "common/system.h"
#include "common/config-manager.h"

namespace Kyra {
KyraEngine_v3::KyraEngine_v3(OSystem *system) : KyraEngine(system) {
	_soundDigital = 0;
	_musicSoundChannel = -1;
	_menuAudioFile = "TITLE1.AUD";
}

KyraEngine_v3::~KyraEngine_v3() {
	delete _soundDigital;
}

int KyraEngine_v3::setupGameFlags() {
	_game = GI_KYRA3;
	_lang = 0;
	Common::Language lang = Common::parseLanguage(ConfMan.get("language"));

	switch (lang) {
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

	return 0;
}

Movie *KyraEngine_v3::createWSAMovie() {
	return new WSAMovieV3(this);
}

int KyraEngine_v3::init() {
	KyraEngine::init();
	
	_soundDigital = new SoundDigital(this, _mixer);
	assert(_soundDigital);
	if (!_soundDigital->init())
		error("_soundDigital->init() failed");
	
	uint32 sz;
	_screen->loadFont(Screen::FID_6_FNT, _res->fileData("6.FNT", &sz));
	_screen->loadFont(Screen::FID_8_FNT, _res->fileData("8FAT.FNT", &sz));
	_screen->setScreenDim(0);
	
	return 0;
}

int KyraEngine_v3::go() {
	_screen->_curPage = 0;
	_screen->clearPage(0);
	
	uint8 pal[768];
	memset(pal, 0, sizeof(pal));
	
	WSAMovieV3 *logo = new WSAMovieV3(this);
	logo->open("REVENGE.WSA", 1, pal);
	assert(logo->opened());
	
	pal[0] = pal[1] = pal[2] = 0;
	
	_screen->setScreenPalette(pal);
	
	// XXX
	playMenuAudioFile();
	
	logo->setX(0); logo->setY(0);
	logo->setDrawPage(0);

	for (int i = 0; i < 64 && !_quitFlag; ++i) {
		uint32 nextRun = _system->getMillis() + 3 * _tickLength;
		logo->displayFrame(i);
		_screen->updateScreen();
		delayUntil(nextRun);
	}

	for (int i = 64; i > 29 && !_quitFlag; --i) {
		uint32 nextRun = _system->getMillis() + 3 * _tickLength;
		logo->displayFrame(i);
		_screen->updateScreen();
		delayUntil(nextRun);
	}
	
	switch (handleMainMenu(logo)) {
	case 0:
		break;
	
	case 1:
		break;
	
	case 2:
		break;
	
	case 3:
		break;
	
	default:
		break;
	}
	
	delete logo;

	return 0;
}

void KyraEngine_v3::playVQA(const char *name) {
	debugC(9, kDebugLevelMain, "KyraEngine::playVQA('%s')", name);
	VQAMovie vqa(this, _system);

	char filename[20];
	int size = 0;		// TODO: Movie size is 0, 1 or 2.

	snprintf(filename, sizeof(filename), "%s%d.VQA", name, size);

	if (vqa.open(filename)) {
		uint8 pal[768];
		memcpy(pal, _screen->getPalette(0), sizeof(pal));
		if (_screen->_curPage == 0)
			_screen->copyCurPageBlock(0, 0, 320, 200, _screen->getPagePtr(3));

		_screen->hideMouse();
		vqa.setDrawPage(0);
		vqa.play();
		vqa.close();
		_screen->showMouse();

		if (_screen->_curPage == 0)
			_screen->copyBlockToPage(0, 0, 0, 320, 200, _screen->getPagePtr(3));
		_screen->setScreenPalette(pal);
	}
}

void KyraEngine_v3::playMenuAudioFile() {
	debugC(9, kDebugLevelMain, "KyraEngine::playMenuAudioFile()");
	if (_soundDigital->isPlaying(_musicSoundChannel))
		return;

	Common::File *handle = new Common::File();
	uint32 temp = 0;
	_res->fileHandle(_menuAudioFile, &temp, *handle);
	if (handle->isOpen()) {
		_musicSoundChannel = _soundDigital->playSound(handle, true, -1);
	}
}

int KyraEngine_v3::handleMainMenu(WSAMovieV3 *logo) {
	debugC(9, kDebugLevelMain, "KyraEngine::handleMainMenu(%p)", (const void*)logo);
	int command = -1;
	
	uint8 colorMap[16];
	memset(colorMap, 0, sizeof(colorMap));
	_screen->setTextColorMap(colorMap);
	
	const char * const *strings = &_mainMenuStrings[_lang << 2];
	Screen::FontId oldFont = _screen->setFont(Screen::FID_8_FNT);
	int charWidthBackUp = _screen->_charWidth;
	
	_screen->setScreenDim(3);
	_screen->copyCurPageBlock(_screen->_curDim->sx, _screen->_curDim->sy, _screen->_curDim->w, _screen->_curDim->h, _screen->getPagePtr(3));

	int x = _screen->_curDim->sx << 3;
	int y = _screen->_curDim->sy;
	int width = _screen->_curDim->w << 3;
	int height =  _screen->_curDim->h;

	drawMainBox(x, y, width, height, 1);
	drawMainBox(x + 1, y + 1, width - 2, height - 2, 0);
	
	int curFrame = 29, frameAdd = 1;
	uint32 nextRun = 0;
	
	drawMainMenu(strings);
	_system->warpMouse(300, 180);
	
	while (command == -1 && !_quitFlag) {
		// yes 2 * _tickLength here not 3 * like in the first draw
		nextRun = _system->getMillis() + 2 * _tickLength;
		logo->displayFrame(curFrame);
		_screen->updateScreen();
		
		curFrame += frameAdd;
		if (curFrame < 29) {
			curFrame = 29;
			frameAdd = 1;
		} else if (curFrame > 63) {
			curFrame = 64;
			frameAdd = -1;
		}
		
		// XXX
		
		while (_system->getMillis() < nextRun) {
			// XXX
			_screen->updateScreen();
			if ((int32)nextRun - (int32)_system->getMillis() >= 10)
				delay(10);
		}
	}
	
	if (_quitFlag)
		command = -1;
	
	_screen->copyBlockToPage(_screen->_curPage, _screen->_curDim->sx, _screen->_curDim->sy, _screen->_curDim->w, _screen->_curDim->h, _screen->getPagePtr(3));
	_screen->_charWidth = charWidthBackUp;
	_screen->setFont(oldFont);
	
	if (command == 3) {
		_screen->fadeToBlack();
		_soundDigital->stopSound(_musicSoundChannel);
		_musicSoundChannel = -1;
	}
	
	return command;
}

void KyraEngine_v3::drawMainMenu(const char * const *strings) {
	debugC(9, kDebugLevelMain, "KyraEngine::drawMainMenu(%p)", (const void*)strings);
	static const uint16 menuTable[] = { 0x01, 0x04, 0x0C, 0x04, 0x00, 0x80, 0xFF, 0x00, 0x01, 0x02, 0x03 };
	
	int top = _screen->_curDim->sy;
	top += menuTable[1];
	
	for (int i = 0; i < menuTable[3]; ++i) {
		int curY = top + i * _screen->getFontHeight();
		gui_printString(strings[i], ((_screen->_curDim->w >> 1) + _screen->_curDim->sx) << 3, curY, menuTable[5], 0, 5);
	}
}

void KyraEngine_v3::drawMainBox(int x, int y, int w, int h, int fill) {
	debugC(9, kDebugLevelMain, "KyraEngine::drawMainBox(%d, %d, %d, %d, %d)", x, y, w, h, fill);
	static const uint8 colorTable[] = { 0x16, 0x19, 0x1A, 0x16 };
	--w; --h;

	if (fill) {
		_screen->fillRect(x, y, x+w, y+h, colorTable[0]);
	}
	
	_screen->drawClippedLine(x, y+h, x+w, y+h, colorTable[1]);
	_screen->drawClippedLine(x+w, y, x+w, y+h, colorTable[1]);
	_screen->drawClippedLine(x, y, x+w, y, colorTable[2]);
	_screen->drawClippedLine(x, y, x, y+h, colorTable[2]);
	
	_screen->setPagePixel(_screen->_curPage, x, y+h, colorTable[3]);
	_screen->setPagePixel(_screen->_curPage, x+w, y, colorTable[3]);
}

void KyraEngine_v3::gui_printString(const char *format, int x, int y, int col1, int col2, int flags, ...) {
	debugC(9, kDebugLevelMain, "KyraEngine::gui_printString('%s', %d, %d, %d, %d, %d, ...)", format, x, y, col1, col2, flags);
	if (!format)
		return;
	
	char string[512];
	va_list vaList;
	va_start(vaList, flags);
	vsprintf(string, format, vaList);
	va_end(vaList);
	
	if (flags & 1) {
		x -= _screen->getTextWidth(string) >> 1;
	}
	
	if (flags & 2) {
		x -= _screen->getTextWidth(string);
	}
	
	if (flags & 4) {
		_screen->printText(string, x - 1, y, 240, col2);
		_screen->printText(string, x, y + 1, 240, col2);
	}
	
	if (flags & 8) {
		_screen->printText(string, x - 1, y, 227, col2);
		_screen->printText(string, x, y + 1, 227, col2);
	}
	
	_screen->printText(string, x, y, col1, col2);
}

} // end of namespace Kyra
