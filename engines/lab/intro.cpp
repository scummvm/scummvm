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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/file.h"

#include "lab/lab.h"

#include "lab/anim.h"
#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/intro.h"
#include "lab/music.h"
#include "lab/resource.h"
#include "lab/utils.h"

namespace Lab {

Intro::Intro(LabEngine *vm) : _vm(vm) {
	_quitIntro = false;
	_font = _vm->_resource->getFont("F:Map.fon");
}

Intro::~Intro() {
	_vm->_graphics->freeFont(&_font);
}

void Intro::introEatMessages() {
	while (1) {
		IntuiMessage *msg = _vm->_event->getMsg();

		if (_vm->shouldQuit()) {
			_quitIntro = true;
			return;
		}

		if (!msg)
			return;

		if ((msg->_msgClass == kMessageRightClick)
		 || ((msg->_msgClass == kMessageRawKey) && (msg->_code == Common::KEYCODE_ESCAPE)))
			_quitIntro = true;
	}
}

void Intro::doPictText(const Common::String filename, bool isScreen) {
	Common::String path = Common::String("Lab:rooms/Intro/") + filename;

	uint timeDelay = (isScreen) ? 35 : 7;
	_vm->updateEvents();

	if (_quitIntro)
		return;

	uint32 lastMillis = 0;
	bool drawNextText = true;
	bool doneFl = false;
	bool begin = true;

	Common::File *textFile = _vm->_resource->openDataFile(path);
	char *textBuffer = new char[textFile->size()];
	textFile->read(textBuffer, textFile->size());
	delete textFile;
	const char *curText = textBuffer;

	while (1) {
		if (drawNextText) {
			if (begin)
				begin = false;
			else if (isScreen)
				_vm->_graphics->fade(false);

			if (isScreen) {
				_vm->_graphics->rectFillScaled(10, 10, 310, 190, 7);

				curText += _vm->_graphics->flowText(_font, _vm->_isHiRes ? 0 : -1, 5, 7, false, false, true, true, _vm->_utils->vgaRectScale(14, 11, 306, 189), curText);
				_vm->_graphics->fade(true);
			} else
				curText += _vm->_graphics->longDrawMessage(Common::String(curText), false);

			doneFl = (*curText == 0);

			drawNextText = false;
			introEatMessages();

			if (_quitIntro) {
				if (isScreen)
					_vm->_graphics->fade(false);

				delete[] textBuffer;
				return;
			}

			lastMillis = _vm->_system->getMillis();
		}

		IntuiMessage *msg = _vm->_event->getMsg();
		if (_vm->shouldQuit()) {
			_quitIntro = true;
			return;
		}

		if (!msg) {
			_vm->updateEvents();
			_vm->_anim->diffNextFrame();

			uint32 elapsedSeconds = (_vm->_system->getMillis() - lastMillis) / 1000;

			if (elapsedSeconds > timeDelay) {
				if (doneFl) {
					if (isScreen)
						_vm->_graphics->fade(false);

					delete[] textBuffer;
					return;
				} else {
					drawNextText = true;
				}
			}
			_vm->waitTOF();
		} else {
			uint32 msgClass = msg->_msgClass;
			uint16 code = msg->_code;

			if ((msgClass == kMessageRightClick) ||
				((msgClass == kMessageRawKey) && (code == Common::KEYCODE_ESCAPE))) {
				_quitIntro = true;

				if (isScreen)
					_vm->_graphics->fade(false);

				delete[] textBuffer;
				return;
			} else if ((msgClass == kMessageLeftClick) || (msgClass == kMessageRightClick)) {
				if (msgClass == kMessageLeftClick) {
					if (doneFl) {
						if (isScreen)
							_vm->_graphics->fade(false);

						delete[] textBuffer;
						return;
					} else
						drawNextText = true;
				}

				introEatMessages();

				if (_quitIntro) {
					if (isScreen)
						_vm->_graphics->fade(false);

					delete[] textBuffer;
					return;
				}
			}

			if (doneFl) {
				if (isScreen)
					_vm->_graphics->fade(false);

				delete[] textBuffer;
				return;
			} else
				drawNextText = true;
		}
	}	// while(1)
}

void Intro::nReadPict(const Common::String filename, bool playOnce, bool noPalChange, bool doBlack, int wait) {
	Common::String finalFileName = Common::String("P:Intro/") + filename;

	_vm->updateEvents();
	introEatMessages();

	if (_quitIntro)
		return;

	if (noPalChange)
		_vm->_anim->_noPalChange = true;

	_vm->_anim->_doBlack = doBlack;
	_vm->_anim->stopDiffEnd();
	_vm->_graphics->readPict(finalFileName, playOnce);

	if (wait) {
		for (int i = 0; i < wait / 10; i++) {
			_vm->updateEvents();
			introEatMessages();
			if (_quitIntro)
				break;
			_vm->_system->delayMillis(10);
		}
	}

	if (noPalChange)
		_vm->_anim->_noPalChange = false;
}

void Intro::play() {
	uint16 palette[16] = {
		0x0000, 0x0855, 0x0FF9, 0x0EE7,
		0x0ED5, 0x0DB4, 0x0CA2, 0x0C91,
		0x0B80, 0x0B80, 0x0B91, 0x0CA2,
		0x0CB3, 0x0DC4, 0x0DD6, 0x0EE7
	};

	if (_vm->getPlatform() == Common::kPlatformDOS) {
		nReadPict("EA0");
		nReadPict("EA1");
		nReadPict("EA2");
		nReadPict("EA3");
	} else if (_vm->getPlatform() == Common::kPlatformWindows) {
		nReadPict("WYRMKEEP", true, false, false, 4000);
	}

	_vm->_graphics->blackAllScreen();
	_vm->_music->resetMusic(false);

	if (_vm->getPlatform() == Common::kPlatformDOS)
		nReadPict("TNDcycle.pic", true, true);
	else
		nReadPict("TNDcycle2.pic", true, true);

	_vm->_graphics->_fadePalette = palette;

	for (int i = 0; i < 16; i++) {
		palette[i] = ((_vm->_anim->_diffPalette[i * 3] >> 2) << 8) +
					((_vm->_anim->_diffPalette[i * 3 + 1] >> 2) << 4) +
					(_vm->_anim->_diffPalette[i * 3 + 2] >> 2);
	}

	_vm->updateEvents();
	introEatMessages();
	if (!_quitIntro)
		_vm->_graphics->fade(true);

	for (int times = 0; times < 150; times++) {
		_vm->updateEvents();
		introEatMessages();
		if (_quitIntro)
			break;

		uint16 temp = palette[2];

		for (int i = 2; i < 15; i++)
			palette[i] = palette[i + 1];

		palette[15] = temp;

		_vm->_graphics->setAmigaPal(palette);
		_vm->waitTOF();
		_vm->waitTOF();
	}

	if (!_quitIntro) {
		_vm->_graphics->fade(false);
		_vm->_graphics->blackAllScreen();
		_vm->updateEvents();
		introEatMessages();
	}

	nReadPict("Title.A");
	nReadPict("AB", true, false, false, 1000);
	nReadPict("BA");
	nReadPict("AC", true, false, false, 1000);
	nReadPict("CA");
	nReadPict("AD", true, false, false, 1000);
	nReadPict("DA");

	_vm->_graphics->blackAllScreen();
	_vm->updateEvents();
	introEatMessages();

	nReadPict("Intro.1", true, true);

	for (int i = 0; i < 16; i++) {
		palette[i] = ((_vm->_anim->_diffPalette[i * 3] >> 2) << 8) +
					((_vm->_anim->_diffPalette[i * 3 + 1] >> 2) << 4) +
					(_vm->_anim->_diffPalette[i * 3 + 2] >> 2);
	}

	doPictText("i.1", true);
	if (_vm->getPlatform() == Common::kPlatformWindows) {
		doPictText("i.2A", true);
		doPictText("i.2B", true);
	}

	_vm->_graphics->blackAllScreen();
	_vm->updateEvents();
	introEatMessages();

	nReadPict("Station1", true, false, true);
	doPictText("i.3");

	nReadPict("Station2", true, false, true);
	doPictText("i.4");

	nReadPict("Stiles4", true, false, true);
	doPictText("i.5");

	nReadPict("Stiles3", true, false, true);
	doPictText("i.6");

	if (_vm->getPlatform() == Common::kPlatformWindows)
		nReadPict("Platform2", true, false, true);
	else
		nReadPict("Platform", true, false, true);
	doPictText("i.7");

	nReadPict("Subway.1", true, false, true);
	doPictText("i.8");

	nReadPict("Subway.2", true, false, true);

	doPictText("i.9");
	doPictText("i.10");
	doPictText("i.11");

	for (int i = 0; i < 50; i++) {
		_vm->updateEvents();
		introEatMessages();
		if (_quitIntro)
			break;

		for (int idx = (8 * 3); idx < (255 * 3); idx++)
			_vm->_anim->_diffPalette[idx] = 255 - _vm->_anim->_diffPalette[idx];

		_vm->waitTOF();
		_vm->_graphics->setPalette(_vm->_anim->_diffPalette, 256);
		_vm->waitTOF();
		_vm->waitTOF();
	}

	doPictText("i.12");
	doPictText("i.13");

	nReadPict("Daed0");
	doPictText("i.14");

	nReadPict("Daed1");
	doPictText("i.15");

	nReadPict("Daed2");
	doPictText("i.16");
	doPictText("i.17");
	doPictText("i.18");

	nReadPict("Daed3");
	doPictText("i.19");
	doPictText("i.20");

	nReadPict("Daed4");
	doPictText("i.21");

	nReadPict("Daed5");
	doPictText("i.22");
	doPictText("i.23");
	doPictText("i.24");

	nReadPict("Daed6");
	doPictText("i.25");
	doPictText("i.26");

	nReadPict("Daed7", false);
	doPictText("i.27");
	doPictText("i.28");

	nReadPict("Daed8");
	doPictText("i.29");
	doPictText("i.30");

	nReadPict("Daed9");
	doPictText("i.31");
	doPictText("i.32");
	doPictText("i.33");

	nReadPict("Daed9a");
	nReadPict("Daed10");
	doPictText("i.34");
	doPictText("i.35");
	doPictText("i.36");

	nReadPict("SubX");

	if (_quitIntro) {
		_vm->_graphics->rectFill(0, 0, _vm->_graphics->_screenWidth - 1, _vm->_graphics->_screenHeight - 1, 0);
		_vm->_anim->_doBlack = true;
	}
}

} // End of namespace Lab
