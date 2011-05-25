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
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/system.h"
#include "common/textconsole.h"

#include "hugo/hugo.h"
#include "hugo/intro.h"
#include "hugo/util.h"
#include "hugo/display.h"
#include "hugo/sound.h"
#include "hugo/text.h"

namespace Hugo {

IntroHandler::IntroHandler(HugoEngine *vm) : _vm(vm), _introX(0), _introY(0) {
	_introXSize = 0;
}

IntroHandler::~IntroHandler() {
}

byte IntroHandler::getIntroSize() const {
	return _introXSize;
}

/**
 * Read _introX and _introY from hugo.dat
 */
void IntroHandler::loadIntroData(Common::SeekableReadStream &in) {
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		int numRows = in.readUint16BE();
		if (varnt == _vm->_gameVariant) {
			_introXSize = numRows;
			_introX = (byte *)malloc(sizeof(byte) * _introXSize);
			_introY = (byte *)malloc(sizeof(byte) * _introXSize);
			for (int i = 0; i < _introXSize; i++) {
				_introX[i] = in.readByte();
				_introY[i] = in.readByte();
			}
		} else {
			in.skip(numRows * 2);
		}
	}
}

void IntroHandler::freeIntroData() {
	free(_introX);
	free(_introY);
}

intro_v1d::intro_v1d(HugoEngine *vm) : IntroHandler(vm) {
}

intro_v1d::~intro_v1d() {
}

void intro_v1d::preNewGame() {
}

void intro_v1d::introInit() {
	_introState = 0;
	introTicks = 0;
	surf.w = 320;
	surf.h = 200;
	surf.pixels = _vm->_screen->getFrontBuffer();
	surf.pitch = 320;
	surf.format = Graphics::PixelFormat::createFormatCLUT8();
	_vm->_screen->displayList(kDisplayInit);
}

bool intro_v1d::introPlay() {
	byte introSize = getIntroSize();

	if (_vm->getGameStatus().skipIntroFl)
		return true;

	if (introTicks < introSize) {
		switch (_introState++) {
		case 0:
			_vm->_screen->drawRectangle(true, 0, 0, 319, 199, _TMAGENTA);
			_vm->_screen->drawRectangle(true, 10, 10, 309, 189, _TBLACK);
			break;

		case 1:
			_vm->_screen->drawShape(20, 92,_TLIGHTMAGENTA,_TMAGENTA);
			_vm->_screen->drawShape(250,92,_TLIGHTMAGENTA,_TMAGENTA);

			// TROMAN, size 10-5
			if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 8)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 8");

			char buffer[80];
			if (_vm->_boot.registered == kRegRegistered)
				strcpy(buffer, "Registered Version");
			else if (_vm->_boot.registered == kRegShareware)
				strcpy(buffer, "Shareware Version");
			else if (_vm->_boot.registered == kRegFreeware)
				strcpy(buffer, "Freeware Version");
			else
				error("Unknown registration flag in hugo.bsf: %d", _vm->_boot.registered);

			font.drawString(&surf, buffer, 0, 163, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			font.drawString(&surf, _vm->getCopyrightString(), 0, 176, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);

			if ((*_vm->_boot.distrib != '\0') && (scumm_stricmp(_vm->_boot.distrib, "David P. Gray"))) {
				sprintf(buffer, "Distributed by %s.", _vm->_boot.distrib);
				font.drawString(&surf, buffer, 0, 75, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			}

			// SCRIPT, size 24-16
			strcpy(buffer, "Hugo's");

			if (font.loadFromFON("SCRIPT.FON")) {
				font.drawString(&surf, buffer, 0, 20, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			} else {
				// Workaround: SCRIPT.FON doesn't load properly at the moment
				_vm->_screen->loadFont(2);
				_vm->_screen->writeStr(kCenter, 20, buffer, _TMAGENTA);
			}

			// TROMAN, size 30-24
			if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 24)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 24");

			strcpy(buffer, "House of Horrors !");
			font.drawString(&surf, buffer, 0, 50, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 2:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 14)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 14");

			strcpy(buffer, "S t a r r i n g :");
			font.drawString(&surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 3:
			// TROMAN, size 20-9
			if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 18)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 18");

			strcpy(buffer, "Hugo !");
			font.drawString(&surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 4:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 14)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 14");

			strcpy(buffer, "P r o d u c e d  b y :");
			font.drawString(&surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 5:
			// TROMAN size 16-9
			strcpy(buffer, "David P Gray !");
			font.drawString(&surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 6:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			strcpy(buffer, "D i r e c t e d   b y :");
			font.drawString(&surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 7:
			// TROMAN, size 16-9
			strcpy(buffer, "David P Gray !");
			font.drawString(&surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 8:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			strcpy(buffer, "M u s i c   b y :");
			font.drawString(&surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 9:
			// TROMAN, size 16-9
			strcpy(buffer, "David P Gray !");
			font.drawString(&surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 10:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 20-14
			if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 18)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 18");

			strcpy(buffer, "E n j o y !");
			font.drawString(&surf, buffer, 0, 100, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		}

		_vm->_screen->displayBackground();
		g_system->updateScreen();
		g_system->delayMillis(1000);
	}

	return (++introTicks >= introSize);
}

intro_v2d::intro_v2d(HugoEngine *vm) : IntroHandler(vm) {
}

intro_v2d::~intro_v2d() {
}

void intro_v2d::preNewGame() {
}

void intro_v2d::introInit() {
	_vm->_screen->displayList(kDisplayInit);
	_vm->_file->readBackground(_vm->_numScreens - 1); // display splash screen
	surf.w = 320;
	surf.h = 200;
	surf.pixels = _vm->_screen->getFrontBuffer();
	surf.pitch = 320;
	surf.format = Graphics::PixelFormat::createFormatCLUT8();

	char buffer[128];

	// TROMAN, size 10-5
	if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 8)))
		error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 8");

	if (_vm->_boot.registered)
		sprintf(buffer, "%s  Registered Version", _vm->getCopyrightString());
	else
		sprintf(buffer, "%s  Shareware Version", _vm->getCopyrightString());

	font.drawString(&surf, buffer, 0, 186, 320, _TLIGHTRED, Graphics::kTextAlignCenter);

	if ((*_vm->_boot.distrib != '\0') && (scumm_stricmp(_vm->_boot.distrib, "David P. Gray"))) {
		// TROMAN, size 10-5
		sprintf(buffer, "Distributed by %s.", _vm->_boot.distrib);
		font.drawString(&surf, buffer, 0, 1, 320, _TLIGHTRED, Graphics::kTextAlignCenter);
	}

	_vm->_screen->displayBackground();
	g_system->updateScreen();
	g_system->delayMillis(5000);
}

bool intro_v2d::introPlay() {
	return true;
}

intro_v3d::intro_v3d(HugoEngine *vm) : IntroHandler(vm) {
}

intro_v3d::~intro_v3d() {
}

void intro_v3d::preNewGame() {
}

void intro_v3d::introInit() {
	_vm->_screen->displayList(kDisplayInit);
	_vm->_file->readBackground(_vm->_numScreens - 1); // display splash screen
	surf.w = 320;
	surf.h = 200;
	surf.pixels = _vm->_screen->getFrontBuffer();
	surf.pitch = 320;
	surf.format = Graphics::PixelFormat::createFormatCLUT8();

	char buffer[128];
	if (_vm->_boot.registered)
		sprintf(buffer, "%s  Registered Version", _vm->getCopyrightString());
	else
		sprintf(buffer,"%s  Shareware Version", _vm->getCopyrightString());

	// TROMAN, size 10-5
	if (!font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 8)))
		error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 8");

	font.drawString(&surf, buffer, 0, 190, 320, _TBROWN, Graphics::kTextAlignCenter);

	if ((*_vm->_boot.distrib != '\0') && (scumm_stricmp(_vm->_boot.distrib, "David P. Gray"))) {
		sprintf(buffer, "Distributed by %s.", _vm->_boot.distrib);
		font.drawString(&surf, buffer, 0, 0, 320, _TBROWN, Graphics::kTextAlignCenter);
	}

	_vm->_screen->displayBackground();
	g_system->updateScreen();
	g_system->delayMillis(5000);

	_vm->_file->readBackground(22); // display screen MAP_3d
	_vm->_screen->displayBackground();
	introTicks = 0;
	_vm->_sound->DOSSongPtr = _vm->_sound->DOSIntroSong;
}

/**
 * Hugo 3 - Preamble screen before going into game.  Draws path of Hugo's plane.
 * Called every tick.  Returns TRUE when complete
 */
bool intro_v3d::introPlay() {
	if (_vm->getGameStatus().skipIntroFl)
		return true;

	if (introTicks < getIntroSize()) {
		font.drawString(&surf, ".", _introX[introTicks], _introY[introTicks] - kDibOffY, 320, _TBRIGHTWHITE);
		_vm->_screen->displayBackground();

		// Text boxes at various times
		switch (introTicks) {
		case 4:
			Utils::notifyBox(_vm->_text->getTextIntro(kIntro1));
			break;
		case 9:
			Utils::notifyBox(_vm->_text->getTextIntro(kIntro2));
			break;
		case 35:
			Utils::notifyBox(_vm->_text->getTextIntro(kIntro3));
			break;
		}
	}

	return (++introTicks >= getIntroSize());
}

intro_v1w::intro_v1w(HugoEngine *vm) : IntroHandler(vm) {
}

intro_v1w::~intro_v1w() {
}

void intro_v1w::preNewGame() {
	_vm->getGameStatus().viewState = kViewIntroInit;
}

void intro_v1w::introInit() {
}

bool intro_v1w::introPlay() {
	return true;
}

intro_v2w::intro_v2w(HugoEngine *vm) : IntroHandler(vm) {
}

intro_v2w::~intro_v2w() {
}

void intro_v2w::preNewGame() {
}

void intro_v2w::introInit() {
	_vm->_file->readBackground(_vm->_numScreens - 1); // display splash screen

	_vm->_screen->displayBackground();
	g_system->updateScreen();
	g_system->delayMillis(3000);
}

bool intro_v2w::introPlay() {
	return true;
}

intro_v3w::intro_v3w(HugoEngine *vm) : IntroHandler(vm) {
}

intro_v3w::~intro_v3w() {
}

void intro_v3w::preNewGame() {
}

/**
 * Hugo 3 - show map and set up for introPlay()
 */
void intro_v3w::introInit() {
	_vm->_screen->displayList(kDisplayInit);
	_vm->_file->readBackground(_vm->_numScreens - 1); // display splash screen
	_vm->_screen->displayBackground();
	g_system->updateScreen();
	g_system->delayMillis(3000);
	_vm->_file->readBackground(22); // display screen MAP_3w
	_vm->_screen->displayBackground();
	introTicks = 0;
	_vm->_screen->loadFont(0);
}

/**
 * Hugo 3 - Preamble screen before going into game.  Draws path of Hugo's plane.
 * Called every tick.  Returns TRUE when complete
 */
bool intro_v3w::introPlay() {
	if (_vm->getGameStatus().skipIntroFl)
		return true;

	if (introTicks < getIntroSize()) {
		// Scale viewport x_intro,y_intro to screen (offsetting y)
		_vm->_screen->writeStr(_introX[introTicks], _introY[introTicks] - kDibOffY, "x", _TBRIGHTWHITE);
		_vm->_screen->displayBackground();

		// Text boxes at various times
		switch (introTicks) {
		case 4:
			Utils::notifyBox(_vm->_text->getTextIntro(kIntro1));
			break;
		case 9:
			Utils::notifyBox(_vm->_text->getTextIntro(kIntro2));
			break;
		case 35:
			Utils::notifyBox(_vm->_text->getTextIntro(kIntro3));
			break;
		}
	}

	return (++introTicks >= getIntroSize());
}
} // End of namespace Hugo
