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

#include "audio/mixer.h"
#include "audio/softsynth/pcspk.h"

#include "common/debug-channels.h"
#include "common/events.h"
#include "common/random.h"

#include "agi/preagi/preagi.h"
#include "agi/graphics.h"

namespace Agi {

PreAgiEngine::PreAgiEngine(OSystem *syst, const AGIGameDescription *gameDesc) : AgiBase(syst, gameDesc) {

	// Setup mixer
	syncSoundSettings();

	memset(&_debug, 0, sizeof(struct AgiDebug));

	_speakerHandle = new Audio::SoundHandle();
}

void PreAgiEngine::initialize() {
	initRenderMode();

	_font = new GfxFont(this);
	_gfx = new GfxMgr(this, _font);
	_picture = new PictureMgr(this, _gfx);

	_font->init();

	//_game._vm->_text->charAttrib_Set(15, 0);

	_defaultColor = IDA_DEFAULT;

	//_game._vm->_text->configureScreen(0); // hardcoded

	_gfx->initVideo();

	_speakerStream = new Audio::PCSpeaker(_mixer->getOutputRate());
	_mixer->playStream(Audio::Mixer::kSFXSoundType, _speakerHandle,
	                   _speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	debugC(2, kDebugLevelMain, "Detect game");

	// clear all resources and events
	for (int i = 0; i < MAX_DIRECTORY_ENTRIES; i++) {
		_game.pictures[i].reset();
		_game.sounds[i] = nullptr; // _game.sounds contains pointers now
		_game.dirPic[i].reset();
		_game.dirSound[i].reset();
	}
}

PreAgiEngine::~PreAgiEngine() {
	_mixer->stopHandle(*_speakerHandle);
	delete _speakerStream;
	delete _speakerHandle;

	delete _picture;
	delete _gfx;
	delete _font;
}

int PreAgiEngine::rnd(int max) {
	return (_rnd->getRandomNumber(max - 1) + 1);
}

// Screen functions
void PreAgiEngine::clearScreen(int attr, bool overrideDefault) {
	if (overrideDefault)
		_defaultColor = attr;

	_gfx->clearDisplay((attr & 0xF0) / 0x10);
}

void PreAgiEngine::clearGfxScreen(int attr) {
	_gfx->drawDisplayRect(0, 0, DISPLAY_DEFAULT_WIDTH - 1, IDI_MAX_ROW_PIC * 8 - 1, (attr & 0xF0) / 0x10);
}

// String functions

void PreAgiEngine::drawStr(int row, int col, int attr, const char *buffer) {
	if (attr == kColorDefault)
		attr = _defaultColor;

	byte foreground = attr & 0x0f;
	byte background = attr >> 4;

	// Simplistic CGA and Hercules mapping that handles all text
	// in Mickey and Winnie. Troll text is handled correctly in
	// graphics mode, but the original switched to CGA 16 color
	// text mode for some parts, and we are not doing that.
	switch (_renderMode) {
	case Common::kRenderCGA:
		// Map non-black text to white
		if (foreground != 0) {
			foreground = 3;
		}
		// Map white background to white
		if (background == 15) {
			background = 3;
		}
		break;
	case Common::kRenderHercA:
	case Common::kRenderHercG:
		// Map non-black text to amber/green
		if (foreground != 0) {
			foreground = 1;
		}
		// Map white background to amber/green,
		// all others to black
		if (background == 0x0f) {
			background = 1;
		} else {
			background = 0;
		}
		break;
	default:
		break;
	}

	const int stringLength = (int)strlen(buffer);
	for (int iChar = 0; iChar < stringLength; iChar++) {
		int code = buffer[iChar];

		switch (code) {
		case '\n':
		case '\r': // winnie
		case 0x8D:
			if (++row == 200 / 8) return;
			col = 0;
			break;

		case '|':
			// swap attribute nibbles
			break;

		default:
			_gfx->drawCharacter(row, col, code, foreground, background, false);

			if (++col == 320 / 8) {
				col = 0;
				if (++row == 200 / 8) return;
			}
		}
	}
}

void PreAgiEngine::clearTextArea() {
	int start = IDI_MAX_ROW_PIC;

	if (getGameID() == GID_TROLL)
		start = 21;

	for (int row = start; row < 200 / 8; row++) {
		clearRow(row);
	}
}

void PreAgiEngine::clearRow(int row) {
	drawStr(row, 0, IDA_DEFAULT, "                                        ");   // 40 spaces
}

void PreAgiEngine::printStr(const char *szMsg) {
	clearTextArea();
	drawStr(21, 0, IDA_DEFAULT, szMsg);
	_system->updateScreen();
}

void PreAgiEngine::XOR80(char *buffer) {
	for (size_t i = 0; i < strlen(buffer); i++)
		if (buffer[i] & 0x80)
			buffer[i] ^= 0x80;
}

void PreAgiEngine::printStrXOR(char *szMsg) {
	XOR80(szMsg);
	printStr(szMsg);
}

// Input functions

int PreAgiEngine::getSelection(SelectionTypes type) {
	Common::Event event;

	while (!shouldQuit()) {
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RETURN_TO_LAUNCHER:
			case Common::EVENT_QUIT:
				return 0;
			case Common::EVENT_RBUTTONUP:
				return 0;
			case Common::EVENT_LBUTTONUP:
				if (type == kSelYesNo || type == kSelAnyKey || type == kSelBackspace)
					return 1;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.flags & Common::KBD_NON_STICKY) {
					break;
				}
				switch (event.kbd.keycode) {
				case Common::KEYCODE_y:
					if (type == kSelYesNo)
						return 1;
					break;
				case Common::KEYCODE_n:
					if (type == kSelYesNo)
						return 0;
					break;
				case Common::KEYCODE_ESCAPE:
					if (type == kSelNumber || type == kSelAnyKey)
						return 0;
					break;
				case Common::KEYCODE_1:
				case Common::KEYCODE_2:
				case Common::KEYCODE_3:
				case Common::KEYCODE_4:
				case Common::KEYCODE_5:
				case Common::KEYCODE_6:
				case Common::KEYCODE_7:
				case Common::KEYCODE_8:
				case Common::KEYCODE_9:
					if (type == kSelNumber)
						return event.kbd.keycode - Common::KEYCODE_1 + 1;
					break;
				case Common::KEYCODE_SPACE:
					if (type == kSelSpace)
						return 1;
					break;
				case Common::KEYCODE_BACKSPACE:
					if (type == kSelBackspace)
						return 0;
					break;
				default:
					break;
				}
				if (type == kSelYesNo) {
					return 2;
				} else if (type == kSelNumber) {
					return 10;
				} else if (type == kSelAnyKey || type == kSelBackspace) {
					return 1;
				}
				break;
			default:
				break;
			}
		}
		_system->updateScreen();
		_system->delayMillis(10);
	}
	return 0;
}

void PreAgiEngine::playNote(int16 frequency, int32 length) {
	_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, frequency, length);
	waitForTimer(length);
}

void PreAgiEngine::waitForTimer(int msec_delay) {
	uint32 start_time = _system->getMillis();

	while (_system->getMillis() < start_time + msec_delay) {
		_system->updateScreen();
		_system->delayMillis(10);
	}
}

} // End of namespace Agi
