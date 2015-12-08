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

#include "lab/lab.h"

#include "lab/intro.h"
#include "lab/music.h"
#include "lab/resource.h"

namespace Lab {
Intro::Intro(LabEngine *vm) : _vm(vm) {
	_quitIntro = false;
}

/**
 * Goes through, and responds to all the intuition messages currently in the
 * message queue.
 */
void Intro::introEatMessages() {
	while (1) {
		IntuiMessage *msg = _vm->getMsg();

		if (g_engine->shouldQuit()) {
			_quitIntro = true;
			return;
		}

		if (msg == NULL)
			return;
		else {
			if (((msg->_msgClass == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & msg->_qualifier)) ||
				  ((msg->_msgClass == RAWKEY) && (msg->_code == 27))
				)
				_quitIntro = true;
		}
	}
}

/**
 * Reads in a picture.
 */
void Intro::doPictText(const char *filename, TextFont *msgFont, bool isScreen) {
	char path[50] = "Lab:rooms/Intro/";
	strcat(path, filename);

	uint timeDelay;
	if (isScreen) {
		_vm->_music->updateMusic();
		timeDelay = 35;
	} else {
		_vm->_music->updateMusic();
		timeDelay = 7;
	}

	if (_quitIntro)
		return;

	uint32 lastSecs = 0L, lastMicros = 0L, secs = 0L, micros = 0L;
	IntuiMessage *msg;
	bool drawNextText = true, end = false, begin = true;

	int32 cls, code, Drawn;
	int16 qualifier;

	Common::File *textFile = g_lab->_resource->openDataFile(path);
	byte *textBuffer = new byte[textFile->size()];
	textFile->read(textBuffer, textFile->size());
	delete textFile;
	byte *curText = textBuffer;

	while (1) {
		if (drawNextText) {
			if (begin)
				begin = false;
			else if (isScreen)
				_vm->_graphics->fade(false, 0);

			if (isScreen) {
				_vm->_graphics->setAPen(7);
				_vm->_graphics->rectFillScaled(10, 10, 310, 190);

				Drawn = _vm->_graphics->flowTextScaled(msgFont, (!_vm->_isHiRes) * -1, 5, 7, false, false, true, true, 14, 11, 306, 189, (char *)curText);
				_vm->_graphics->fade(true, 0);
			} else {
				Drawn = _vm->_graphics->longDrawMessage((char *)curText);
			}

			curText += Drawn;

			end = (*curText == 0);

			drawNextText = false;
			introEatMessages();

			if (_quitIntro) {
				if (isScreen)
					_vm->_graphics->fade(false, 0);

				delete[] textBuffer;
				return;
			}

			_vm->getTime(&lastSecs, &lastMicros);
		}

		msg = _vm->getMsg();

		if (msg == NULL) {
			_vm->_music->updateMusic();
			_vm->_anim->diffNextFrame();
			_vm->getTime(&secs, &micros);
			_vm->anyTimeDiff(lastSecs, lastMicros, secs, micros, &secs, &micros);

			if (secs > timeDelay) {
				if (end) {
					if (isScreen)
						_vm->_graphics->fade(false, 0);

					delete[] textBuffer;
					return;
				} else {
					drawNextText = true;
				}
			}

			_vm->waitTOF();
		} else {
			cls       = msg->_msgClass;
			qualifier = msg->_qualifier;
			code      = msg->_code;

			if (((cls == MOUSEBUTTONS) && (IEQUALIFIER_RBUTTON & qualifier)) ||
				  ((cls == RAWKEY) && (code == 27))) {
				_quitIntro = true;

				if (isScreen)
					_vm->_graphics->fade(false, 0);

				delete[] textBuffer;
				return;
			}

			else if (cls == MOUSEBUTTONS) {
				if (IEQUALIFIER_LEFTBUTTON & qualifier) {
					if (end) {
						if (isScreen)
							_vm->_graphics->fade(false, 0);

						delete[] textBuffer;
						return;
					} else
						drawNextText = true;
				}

				introEatMessages();

				if (_quitIntro) {
					if (isScreen)
						_vm->_graphics->fade(false, 0);

					delete[] textBuffer;
					return;
				}
			}

			if (end) {
				if (isScreen)
					_vm->_graphics->fade(false, 0);

				delete[] textBuffer;
				return;
			} else
				drawNextText = true;
		}
	}	// while(1)
}

/**
 * Does a one second delay, but checks the music while doing it.
 */
void Intro::musicDelay() {
	_vm->_music->updateMusic();

	if (_quitIntro)
		return;

	for (uint16 i = 0; i < 20; i++) {
		_vm->_music->updateMusic();
		_vm->waitTOF();
		_vm->waitTOF();
		_vm->waitTOF();
	}
}

void Intro::nReadPict(const char *filename, bool playOnce) {
	Common::String finalFileName = Common::String("P:Intro/") + filename;

	_vm->_music->updateMusic();
	introEatMessages();

	if (_quitIntro)
		return;

	_vm->_anim->_doBlack = _introDoBlack;
	_vm->_anim->stopDiffEnd();
	_vm->_graphics->readPict(finalFileName.c_str(), playOnce);
}

/**
 * Does the introduction sequence for Labyrinth.
 */
void Intro::introSequence() {
	uint16 palette[16] = {
		0x0000, 0x0855, 0x0FF9, 0x0EE7,
		0x0ED5, 0x0DB4, 0x0CA2, 0x0C91,
		0x0B80, 0x0B80, 0x0B91, 0x0CA2,
		0x0CB3, 0x0DC4, 0x0DD6, 0x0EE7
	};

	_vm->_anim->_doBlack = true;

	if (_vm->getPlatform() != Common::kPlatformWindows) {
		nReadPict("EA0", true);
		nReadPict("EA1", true);
		nReadPict("EA2", true);
		nReadPict("EA3", true);
	} else {
		nReadPict("WYRMKEEP", true);
		// Wait 4 seconds
		for (uint16 i = 0; i < 4 * 1000 / 10; i++) {
			introEatMessages();
			if (_quitIntro)
				break;
			g_system->delayMillis(10);
		}
	}

	_vm->_graphics->blackAllScreen();
	_vm->_music->initMusic();

	_vm->_anim->_noPalChange = true;
	if (_vm->getPlatform() != Common::kPlatformWindows)
		nReadPict("TNDcycle.pic", true);
	else
		nReadPict("TNDcycle2.pic", true);
	_vm->_anim->_noPalChange = false;

	_vm->_graphics->FadePalette = palette;

	for (uint16 i = 0; i < 16; i++) {
		if (_quitIntro)
			break;

		palette[i] = ((_vm->_anim->_diffPalette[i * 3] >> 2) << 8) +
					((_vm->_anim->_diffPalette[i * 3 + 1] >> 2) << 4) +
					(_vm->_anim->_diffPalette[i * 3 + 2] >> 2);
	}

	_vm->_music->updateMusic();
	_vm->_graphics->fade(true, 0);

	for (int times = 0; times < 150; times++) {
		if (_quitIntro)
			break;

		_vm->_music->updateMusic();
		uint16 temp = palette[2];

		for (uint16 i = 2; i < 15; i++)
			palette[i] = palette[i + 1];

		palette[15] = temp;

		_vm->_graphics->setAmigaPal(palette, 16);
		_vm->waitTOF();
	}

	_vm->_graphics->fade(false, 0);
	_vm->_graphics->blackAllScreen();
	_vm->_music->updateMusic();

	nReadPict("Title.A", true);
	nReadPict("AB", true);
	musicDelay();
	nReadPict("BA", true);
	nReadPict("AC", true);
	musicDelay();

	if (_vm->getPlatform() == Common::kPlatformWindows)
		musicDelay(); // more credits on this page now

	nReadPict("CA", true);
	nReadPict("AD", true);
	musicDelay();

	if (_vm->getPlatform() == Common::kPlatformWindows)
		musicDelay(); // more credits on this page now

	nReadPict("DA", true);
	musicDelay();

	_vm->_music->updateMusic();
	_vm->_graphics->blackAllScreen();
	_vm->_music->updateMusic();

	TextFont *msgFont = _vm->_resource->getFont("P:Map.fon");

	_vm->_anim->_noPalChange = true;
	nReadPict("Intro.1", true);
	_vm->_anim->_noPalChange = false;

	for (uint16 i = 0; i < 16; i++) {
		palette[i] = ((_vm->_anim->_diffPalette[i * 3] >> 2) << 8) +
					((_vm->_anim->_diffPalette[i * 3 + 1] >> 2) << 4) +
					(_vm->_anim->_diffPalette[i * 3 + 2] >> 2);
	}

	doPictText("i.1", msgFont, true);
	doPictText("i.2A", msgFont, true);
	doPictText("i.2B", msgFont, true);

	_vm->_graphics->blackAllScreen();
	_vm->_music->updateMusic();

	_introDoBlack = true;
	nReadPict("Station1", true);
	doPictText("i.3", msgFont, false);

	nReadPict("Station2", true);
	doPictText("i.4", msgFont, false);

	nReadPict("Stiles4", true);
	doPictText("i.5", msgFont, false);

	nReadPict("Stiles3", true);
	doPictText("i.6", msgFont, false);

	nReadPict("Platform2", true);
	doPictText("i.7", msgFont, false);

	nReadPict("Subway.1", true);
	doPictText("i.8", msgFont, false);

	nReadPict("Subway.2", true);

	doPictText("i.9", msgFont, false);
	doPictText("i.10", msgFont, false);
	doPictText("i.11", msgFont, false);

	if (!_quitIntro)
		for (uint16 i = 0; i < 50; i++) {
			for (uint16 idx = (8 * 3); idx < (255 * 3); idx++)
				_vm->_anim->_diffPalette[idx] = 255 - _vm->_anim->_diffPalette[idx];

			_vm->_music->updateMusic();
			_vm->waitTOF();
			_vm->_graphics->setPalette(_vm->_anim->_diffPalette, 256);
			_vm->waitTOF();
			_vm->waitTOF();
		}

	doPictText("i.12", msgFont, false);
	doPictText("i.13", msgFont, false);

	_introDoBlack = false;
	nReadPict("Daed0", true);
	doPictText("i.14", msgFont, false);

	nReadPict("Daed1", true);
	doPictText("i.15", msgFont, false);

	nReadPict("Daed2", true);
	doPictText("i.16", msgFont, false);
	doPictText("i.17", msgFont, false);
	doPictText("i.18", msgFont, false);

	nReadPict("Daed3", true);
	doPictText("i.19", msgFont, false);
	doPictText("i.20", msgFont, false);

	nReadPict("Daed4", true);
	doPictText("i.21", msgFont, false);

	nReadPict("Daed5", true);
	doPictText("i.22", msgFont, false);
	doPictText("i.23", msgFont, false);
	doPictText("i.24", msgFont, false);

	nReadPict("Daed6", true);
	doPictText("i.25", msgFont, false);
	doPictText("i.26", msgFont, false);

	nReadPict("Daed7", false);
	doPictText("i.27", msgFont, false);
	doPictText("i.28", msgFont, false);
	_vm->_anim->stopDiffEnd();

	nReadPict("Daed8", true);
	doPictText("i.29", msgFont, false);
	doPictText("i.30", msgFont, false);

	nReadPict("Daed9", true);
	doPictText("i.31", msgFont, false);
	doPictText("i.32", msgFont, false);
	doPictText("i.33", msgFont, false);

	nReadPict("Daed9a", true);
	nReadPict("Daed10", true);
	doPictText("i.34", msgFont, false);
	doPictText("i.35", msgFont, false);
	doPictText("i.36", msgFont, false);

	nReadPict("SubX", true);

	if (_quitIntro) {
		_vm->_graphics->setAPen(0);
		_vm->_graphics->rectFill(0, 0, _vm->_graphics->_screenWidth - 1, _vm->_graphics->_screenHeight - 1);
		_vm->_anim->_doBlack = true;
	}

	_vm->_graphics->closeFont(msgFont);
}

} // End of namespace Lab
