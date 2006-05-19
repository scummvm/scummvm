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

#include "common/system.h"

namespace Kyra {
KyraEngine_v3::KyraEngine_v3(OSystem *system) : KyraEngine(system) {
	_soundDigital = 0;
	_musicSoundChannel = -1;
	_menuAudioFile = "TITLE1.AUD";
}

KyraEngine_v3::~KyraEngine_v3() {
	delete _soundDigital;
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

	for (int i = 0; i < 64; ++i) {
		uint32 nextRun = _system->getMillis() + 3 * _tickLength;
		logo->displayFrame(i);
		_screen->updateScreen();
		delayUntil(nextRun);
	}

	for (int i = 64; i > 29; --i) {
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

void KyraEngine_v3::playMenuAudioFile() {
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
	int command = -1;
	
	const char * const *strings = &_mainMenuStrings[/*_lang*4*/0];
	Screen::FontId oldFont = _screen->setFont(Screen::FID_8_FNT);
	int charWidthBackUp = _screen->_charWidth;
	
	_screen->setScreenDim(3);
	_screen->copyCurPageBlock(_screen->_curDim->sx, _screen->_curDim->sy, _screen->_curDim->w, _screen->_curDim->h, _screen->getPagePtr(3));

//	int left = _screen->_curDim->sx << 3;
//	int top = _screen->_curDim->sy;
//	int right = left + (_screen->_curDim->w << 3);
//	int bottom = top + _screen->_curDim->h;

	// XXX
	// gui_unk1(left, top, right, bottom, 1, 0);
	// gui_unk1(left + 1, top + 1, right - 1, bottom - 1, 0, 0);
	
	int curFrame = 29, frameAdd = 1;
	uint32 nextRun = 0;
	
	drawMainMenu(strings, 0xFFFF);
	
	while (command == -1) {
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

void KyraEngine_v3::drawMainMenu(const char * const *strings, int unk1) {
#if 0
	static const uint16 _menuTable[] = { 0x01, 0x04, 0x0C, 0x04, 0x00, 0xFF, 0x00, 0x01, 0x02, 0x03 };
	
	int left = _screen->_curDim->sx << 3; int top = _screen->_curDim->sy;
	int width = _screen->_curDim->w << 3; int height = _screen->_curDim->h;
	
	top += _menuTable[1];
	
	for (int i = 0; i < _menuTable[3]; ++i) {
		int curY = top + i * _screen->getFontHeight();
		// XXX
	}
#endif
}

void KyraEngine_v3::playVQA(const char *filename) {
	VQAMovie vqa(this, _system);

	uint8 pal[768];
	memcpy(pal, _screen->_currentPalette, sizeof(pal));
	if (_screen->_curPage == 0)
		_screen->copyCurPageBlock(0, 0, 320, 200, _screen->getPagePtr(3));

	vqa.open(filename, 0, NULL);
	if (vqa.opened()) {
		vqa.setX(0); vqa.setY(0);
		vqa.setDrawPage(0);
		vqa.play();
		vqa.close();
	}

	if (_screen->_curPage == 0)
		_screen->copyBlockToPage(0, 0, 0, 320, 200, _screen->getPagePtr(3));
	_screen->setScreenPalette(pal);
}

} // end of namespace Kyra
