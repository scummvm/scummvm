/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/system.h"
#include "common/events.h"
#include "common/textconsole.h"
#include "graphics/font.h"
#include "graphics/pixelformat.h"

#include "hugo/hugo.h"
#include "hugo/intro.h"
#include "hugo/file.h"
#include "hugo/game.h"
#include "hugo/util.h"
#include "hugo/display.h"
#include "hugo/sound.h"
#include "hugo/text.h"

namespace Hugo {

IntroHandler::IntroHandler(HugoEngine *vm) : _vm(vm) {
	_introXSize = 0;
	_introTicks = 0;
	_introX = _introY = nullptr;
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
	_introX = _introY = nullptr;
}

/**
 * Wait for a delay in milliseconds while processing events.
 * This keeps the program window responsive during long delays.
 * Returns false if interrupted by a quit event or the Escape key.
 */
bool IntroHandler::wait(uint32 delay) {
	const uint32 startTime = _vm->_system->getMillis();

	while (!_vm->shouldQuit()) {
		Common::Event event;
		while (_vm->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RETURN_TO_LAUNCHER:
			case Common::EVENT_QUIT:
				return false; // interrupted by quit

			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionEscape) {
					return false; // interrupted by Escape
				}
				break;

			default:
				break;
			}
		}

		if (_vm->_system->getMillis() - startTime >= delay) {
			return true; // delay completed
		}

		_vm->_system->updateScreen();
		_vm->_system->delayMillis(10);
	}

	return false; // interrupted by quit
}

intro_v1d::intro_v1d(HugoEngine *vm) : IntroHandler(vm) {
	_introState = 0;
}

intro_v1d::~intro_v1d() {
}

void intro_v1d::preNewGame() {
}

void intro_v1d::introInit() {
	_introState = 0;
	_introTicks = 0;
	_surf.init(320, 200, 320, _vm->_screen->getFrontBuffer(), Graphics::PixelFormat::createFormatCLUT8());
	_vm->_screen->displayList(kDisplayInit);
}

bool intro_v1d::introPlay() {
	byte introSize = getIntroSize();

	if (_vm->getGameStatus()._skipIntroFl)
		return true;

#ifdef USE_TTS
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan && ttsMan->isSpeaking()) {
		return false;
	}

	Common::String ttsMessage;
#endif

	Common::String copyright;

	if (_introTicks < introSize) {
		switch (_introState++) {
		case 0:
			_vm->_screen->drawRectangle(true, 0, 0, 319, 199, _TMAGENTA);
			_vm->_screen->drawRectangle(true, 10, 10, 309, 189, _TBLACK);
			break;

		case 1:
			_vm->_screen->drawShape(20, 92,_TLIGHTMAGENTA,_TMAGENTA);
			_vm->_screen->drawShape(250,92,_TLIGHTMAGENTA,_TMAGENTA);

			// TROMAN, size 10-5
			if (!_font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 8)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 8");

			char buffer[80];
			if (_vm->_boot._registered == kRegRegistered)
				Common::strcpy_s(buffer, "Registered Version");
			else if (_vm->_boot._registered == kRegShareware)
				Common::strcpy_s(buffer, "Shareware Version");
			else if (_vm->_boot._registered == kRegFreeware)
				Common::strcpy_s(buffer, "Freeware Version");
			else
				error("Unknown registration flag in hugo.bsf: %d", _vm->_boot._registered);

			_font.drawString(&_surf, buffer, 0, 163, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			copyright = Common::String::format("%s %s", _vm->getCopyrightString1(), _vm->getCopyrightString2());
			_font.drawString(&_surf, copyright, 0, 176, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);

#ifdef USE_TTS
			ttsMessage = "Hugo's House of Horrors\n\n";
			ttsMessage += buffer;
			ttsMessage += '\n';
			ttsMessage += _vm->getCopyrightString1();
			ttsMessage += ' ';
			ttsMessage += _vm->getCopyrightString2();
#endif

			if ((*_vm->_boot._distrib != '\0') && (scumm_stricmp(_vm->_boot._distrib, "David P. Gray"))) {
				Common::sprintf_s(buffer, "Distributed by %s.", _vm->_boot._distrib);
				_font.drawString(&_surf, buffer, 0, 75, 320, _TMAGENTA, Graphics::kTextAlignCenter);
#ifdef USE_TTS
				// Insert at second newline after "Hugo's House of Horrors" so that a newline remains between that line
				// and this one
				ttsMessage.insertString(buffer, ttsMessage.find('\n') + 1);
#endif
			}

			// SCRIPT, size 24-16
			Common::strcpy_s(buffer, "Hugo's");

			if (_font.loadFromFON("SCRIPT.FON")) {
				_font.drawString(&_surf, buffer, 0, 20, 320, _TMAGENTA, Graphics::kTextAlignCenter);
			} else {
				// Workaround: SCRIPT.FON doesn't load properly at the moment
				_vm->_screen->loadFont(2);
				_vm->_screen->writeStr(kCenter, 20, buffer, _TMAGENTA);
			}

			// TROMAN, size 30-24
			if (!_font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 24)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 24");

			Common::strcpy_s(buffer, "House of Horrors !");
			_font.drawString(&_surf, buffer, 0, 50, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
			break;
		case 2:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			if (!_font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 14)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 14");

			Common::strcpy_s(buffer, "S t a r r i n g :");
			_font.drawString(&_surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
#ifdef USE_TTS
			ttsMessage = "Starring:";
#endif
			break;
		case 3:
			// TROMAN, size 20-9
			if (!_font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 18)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 18");

			Common::strcpy_s(buffer, "Hugo !");
			_font.drawString(&_surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
#ifdef USE_TTS
			ttsMessage = buffer;
#endif
			break;
		case 4:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			if (!_font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 14)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 14");

			Common::strcpy_s(buffer, "P r o d u c e d  b y :");
			_font.drawString(&_surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
#ifdef USE_TTS
			ttsMessage = "Produced by:";
#endif
			break;
		case 5:
			// TROMAN size 16-9
			Common::strcpy_s(buffer, "David P Gray !");
			_font.drawString(&_surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
#ifdef USE_TTS
			ttsMessage = buffer;
#endif
			break;
		case 6:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			Common::strcpy_s(buffer, "D i r e c t e d   b y :");
			_font.drawString(&_surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
#ifdef USE_TTS
			ttsMessage = "Directed by:";
#endif
			break;
		case 7:
			// TROMAN, size 16-9
			Common::strcpy_s(buffer, "David P Gray !");
			_font.drawString(&_surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
#ifdef USE_TTS
			ttsMessage = buffer;
#endif
			break;
		case 8:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 16-9
			Common::strcpy_s(buffer, "M u s i c   b y :");
			_font.drawString(&_surf, buffer, 0, 95, 320, _TMAGENTA, Graphics::kTextAlignCenter);
#ifdef USE_TTS
			ttsMessage = "Music by:";
#endif
			break;
		case 9:
			// TROMAN, size 16-9
			Common::strcpy_s(buffer, "David P Gray !");
			_font.drawString(&_surf, buffer, 0, 115, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
#ifdef USE_TTS
			ttsMessage = buffer;
#endif
			break;
		case 10:
			_vm->_screen->drawRectangle(true, 82, 92, 237, 138, _TBLACK);

			// TROMAN, size 20-14
			if (!_font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 18)))
				error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 18");

			Common::strcpy_s(buffer, "E n j o y !");
			_font.drawString(&_surf, buffer, 0, 100, 320, _TLIGHTMAGENTA, Graphics::kTextAlignCenter);
#ifdef USE_TTS
			ttsMessage = "Enjoy!";
#endif
			break;
		default:
			break;
		}

#ifdef USE_TTS
		_vm->sayText(ttsMessage);
#endif

		_vm->_screen->displayBackground();
		if (!wait(1000)) {
			// Wait was interrupted by quit event or Escape.
			// Skip the rest of the introduction.
			return true;
		}
	}

	return (++_introTicks >= introSize);
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
	_surf.init(320, 200, 320, _vm->_screen->getFrontBuffer(), Graphics::PixelFormat::createFormatCLUT8());

	char buffer[128];

	// TROMAN, size 10-5
	if (!_font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 8)))
		error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 8");

	if (_vm->_boot._registered)
		Common::sprintf_s(buffer, "%s %s  Registered Version", _vm->getCopyrightString1(), _vm->getCopyrightString2());
	else
		Common::sprintf_s(buffer, "%s %s  Shareware Version", _vm->getCopyrightString1(), _vm->getCopyrightString2());

	_font.drawString(&_surf, buffer, 0, 186, 320, _TLIGHTRED, Graphics::kTextAlignCenter);

#ifdef USE_TTS
	Common::String ttsMessage = buffer;
#endif

	if ((*_vm->_boot._distrib != '\0') && (scumm_stricmp(_vm->_boot._distrib, "David P. Gray"))) {
		// TROMAN, size 10-5
		Common::sprintf_s(buffer, "Distributed by %s.", _vm->_boot._distrib);
		_font.drawString(&_surf, buffer, 0, 1, 320, _TLIGHTRED, Graphics::kTextAlignCenter);

#ifdef USE_TTS
		ttsMessage = buffer + ttsMessage;
#endif
	}

#ifdef USE_TTS
	_vm->sayText(ttsMessage);
#endif

	_vm->_screen->displayBackground();
	wait(5000);

#ifdef USE_TTS
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	while (ttsMan && ttsMan->isSpeaking()) {
		g_system->delayMillis(10);
	}
#endif
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
	_surf.init(320, 200, 320, _vm->_screen->getFrontBuffer(), Graphics::PixelFormat::createFormatCLUT8());

	char buffer[128];
	if (_vm->_boot._registered)
		Common::sprintf_s(buffer, "%s %s  Registered Version", _vm->getCopyrightString1(), _vm->getCopyrightString2());
	else
		Common::sprintf_s(buffer,"%s %s  Shareware Version", _vm->getCopyrightString1(), _vm->getCopyrightString2());

	// TROMAN, size 10-5
	if (!_font.loadFromFON("TMSRB.FON", Graphics::WinFontDirEntry("Tms Rmn", 8)))
		error("Unable to load font TMSRB.FON, face 'Tms Rmn', size 8");

	_font.drawString(&_surf, buffer, 0, 190, 320, _TBROWN, Graphics::kTextAlignCenter);

#ifdef USE_TTS
	Common::String ttsMessage = buffer;
#endif

	if ((*_vm->_boot._distrib != '\0') && (scumm_stricmp(_vm->_boot._distrib, "David P. Gray"))) {
		Common::sprintf_s(buffer, "Distributed by %s.", _vm->_boot._distrib);
		_font.drawString(&_surf, buffer, 0, 0, 320, _TBROWN, Graphics::kTextAlignCenter);

#ifdef USE_TTS
		ttsMessage = buffer + ttsMessage;
#endif
	}

#ifdef USE_TTS
	_vm->sayText(ttsMessage);
#endif

	_vm->_screen->displayBackground();
	wait(5000);

#ifdef USE_TTS
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	while (ttsMan && ttsMan->isSpeaking()) {
		g_system->delayMillis(10);
	}
#endif

	_vm->_file->readBackground(22); // display screen MAP_3d
	_vm->_screen->displayBackground();
	_introTicks = 0;
	_vm->_sound->_DOSSongPtr = _vm->_sound->_DOSIntroSong;
}

/**
 * Hugo 3 - Preamble screen before going into game.  Draws path of Hugo's plane.
 * Called every tick.  Returns TRUE when complete
 */
bool intro_v3d::introPlay() {
	if (_vm->getGameStatus()._skipIntroFl)
		return true;

	if (_introTicks < getIntroSize()) {
		_surf.setPixel(_introX[_introTicks], _introY[_introTicks] - kDibOffY, _TBRIGHTWHITE);
		_vm->_screen->displayBackground();

		// Text boxes at various times
		switch (_introTicks) {
		case 4:
			_vm->notifyBox(_vm->_text->getTextIntro(kIntro1), true);
			break;
		case 9:
			_vm->notifyBox(_vm->_text->getTextIntro(kIntro2), true);
			break;
		case 35:
			_vm->notifyBox(_vm->_text->getTextIntro(kIntro3), true);
			break;
		default:
			break;
		}
	}

	return (++_introTicks >= getIntroSize());
}

intro_v1w::intro_v1w(HugoEngine *vm) : IntroHandler(vm) {
}

intro_v1w::~intro_v1w() {
}

void intro_v1w::preNewGame() {
	_vm->getGameStatus()._viewState = kViewIntroInit;
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
	wait(3000);
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
	wait(3000);
	_vm->_file->readBackground(22); // display screen MAP_3w
	_vm->_screen->displayBackground();
	_introTicks = 0;
	_vm->_screen->loadFont(0);
}

/**
 * Hugo 3 - Preamble screen before going into game.  Draws path of Hugo's plane.
 * Called every tick.  Returns TRUE when complete
 */
bool intro_v3w::introPlay() {
	if (_vm->getGameStatus()._skipIntroFl)
		return true;

	if (_introTicks < getIntroSize()) {
		// Scale viewport x_intro,y_intro to screen (offsetting y)
		_vm->_screen->writeStr(_introX[_introTicks], _introY[_introTicks] - kDibOffY, "x", _TBRIGHTWHITE);
		_vm->_screen->displayBackground();

		// Text boxes at various times
		switch (_introTicks) {
		case 4:
			_vm->notifyBox(_vm->_text->getTextIntro(kIntro1), true);
			break;
		case 9:
			_vm->notifyBox(_vm->_text->getTextIntro(kIntro2), true);
			break;
		case 35:
			_vm->notifyBox(_vm->_text->getTextIntro(kIntro3), true);
			break;
		default:
			break;
		}
	}

	return (++_introTicks >= getIntroSize());
}
} // End of namespace Hugo
