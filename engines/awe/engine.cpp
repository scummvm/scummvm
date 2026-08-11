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

#include "awe/engine.h"
#include "awe/gfx.h"
#include "awe/resource_nth.h"
#include "awe/system_stub.h"

namespace Awe {

static const uint16 RESTART_POS[36 * 2] = {
	16008,  0, 16001,  0, 16002, 10, 16002, 12, 16002, 14,
	16003, 20, 16003, 24, 16003, 26, 16004, 30, 16004, 31,
	16004, 32, 16004, 33, 16004, 34, 16004, 35, 16004, 36,
	16004, 37, 16004, 38, 16004, 39, 16004, 40, 16004, 41,
	16004, 42, 16004, 43, 16004, 44, 16004, 45, 16004, 46,
	16004, 47, 16004, 48, 16004, 49, 16006, 64, 16006, 65,
	16006, 66, 16006, 67, 16006, 68, 16005, 50, 16006, 60,
	16007, 0
};

Engine::Engine(Sound *sound, DataType dataType, int partNum) :
		_sound(sound), _script(sound, &_res, &_ply, &_vid),
		_res(&_vid, dataType), _ply(&_res), _vid(&_res),
		_partNum(partNum) {
	sound->setPlayer(&_ply);
}

void Engine::setSystemStub(SystemStub *stub, Gfx *graphics) {
	_stub = stub;
	_script._stub = stub;
	_graphics = graphics;
}

void Engine::run() {
	switch (_state) {
	case kStateLogo3DO:
		doThreeScreens();
		scrollText(0, 380, Video::NOTE_TEXT_3DO);
		playCinepak("Logo.Cine");
		playCinepak("Spintitle.Cine");
		break;
	case kStateTitle3DO:
		titlePage();
		break;
	case kStateEnd3DO:
		doEndCredits();
		break;
	case kStateGame:
		_script.setupTasks();
		_script.updateInput();
		processInput();
		_script.runTasks();
#if 0
		_mix.update();
#endif
		if (_res.getDataType() == DT_3DO) {
			switch (_res._nextPart) {
			case 16009:
				_state = kStateEnd3DO;
				break;
			case 16000:
				_state = kStateTitle3DO;
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}
}

void Engine::setup(Language lang, int graphicsType) {
	_vid._graphics = _graphics;
	int w = GFX_W;
	int h = GFX_H;
	if (_res.getDataType() != DT_3DO) {
		_vid._graphics->_fixUpPalette = FIXUP_PALETTE_REDRAW;
	}

	_vid.init();

	_res._lang = lang;
	_res.allocMemBlock();
	_res.readEntries();
	_res.dumpEntries();
	const bool isNth = !Gfx::_is1991 && (_res.getDataType() == DT_15TH_EDITION || _res.getDataType() == DT_20TH_EDITION);
	if (isNth) {
		// get HD background bitmaps resolution
		_res._nth->getBitmapSize(&w, &h);
	}
	_graphics->init(w, h);
	if (isNth) {
		_res.loadFont();
		_res.loadHeads();
	} else {
		_vid.setDefaultFont();
	}
	_script.init();

	switch (_res.getDataType()) {
	case DT_DOS:
	case DT_AMIGA:
	case DT_ATARI:
	case DT_ATARI_DEMO:
		switch (lang) {
		case Common::FR_FRA:
			_vid._stringsTable = Video::STRINGS_TABLE_FR;
			break;
		case Common::IT_ITA:
			_vid._stringsTable = Video::STRINGS_TABLE_ITA;
			break;
		case Common::EN_ANY:
		default:
			_vid._stringsTable = Video::STRINGS_TABLE_ENG;
			break;
		}
		break;
	case DT_WIN31:
	case DT_15TH_EDITION:
	case DT_20TH_EDITION:
	case DT_3DO:
		break;
	}

	if (_res._copyProtection) {
		switch (_res.getDataType()) {
		case DT_DOS:
			if (!_res._hasPasswordScreen) {
				break;
			}
			/* fall-through */
		case DT_AMIGA:
		case DT_ATARI:
		case DT_WIN31:
			_partNum = kPartCopyProtection;
			break;
		default:
			break;
		}
	}

	if (_res.getDataType() == DT_3DO && _partNum == kPartIntro) {
		_state = kStateLogo3DO;
	} else {
		_state = kStateGame;
		const int num = _partNum;
		if (num < 36) {
			_script.restartAt(RESTART_POS[num * 2], RESTART_POS[num * 2 + 1]);
		} else {
			_script.restartAt(num);
		}
	}
}

void Engine::finish() {
	_graphics->fini();
	_ply.stop();
	_sound->stopAll();
	_res.freeMemBlock();
}

void Engine::processInput() {
	if (_stub->_pi.fastMode) {
		_script._fastMode = !_script._fastMode;
		_stub->_pi.fastMode = false;
	}
}

void Engine::doThreeScreens() {
	_script.snd_playMusic(1, 0, 0);
	static const int BITMAPS[] = { 67, 68, 69, -1 };

	for (int i = 0; BITMAPS[i] != -1 && !_stub->_pi.quit; ++i) {
		_res.loadBmp(BITMAPS[i]);
		_vid.updateDisplay(0, _stub);
		while (!_stub->_pi.quit) {
			_stub->processEvents();
			if (_stub->_pi.action) {
				_stub->_pi.action = false;
				break;
			}
			_stub->sleep(50);
		}
	}
	_state = kStateTitle3DO;
}

void Engine::doEndCredits() {
	scrollText(0, 380, Video::END_TEXT_3DO);
	_script.snd_playMusic(0, 0, 0);
	playCinepak("ootw2.cine");
	_state = kStateTitle3DO;
}

void Engine::playCinepak(const char *name) {
	warning("STUB: Engine::playCinepak()");
}

void Engine::scrollText(int a, int b, const char *text) {
	warning("STUB: Engine::scrollText()");
}

void Engine::titlePage() {
	_res.loadBmp(70);
	static const int kCursorColor = 0;
	_vid.setPaletteColor(kCursorColor, 255, 0, 0);
	static const uint16 YPOS[] = { 97, 123, 149 };
	int y = 0;

	while (!_stub->_pi.quit) {
		_vid.copyPage(0, 1, 0);
		_vid.drawRect(1, kCursorColor, 97, YPOS[y], 210, YPOS[y + 1]);
		_stub->processEvents();
		if (_stub->_pi.dirMask & PlayerInput::DIR_DOWN) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_DOWN;
			_partNum = kPartPassword;
			y = 1;
		}
		if (_stub->_pi.dirMask & PlayerInput::DIR_UP) {
			_stub->_pi.dirMask &= ~PlayerInput::DIR_UP;
			_partNum = kPartIntro;
			y = 0;
		}
		if (_stub->_pi.action) {
			_stub->_pi.action = false;
			_script.restartAt(_partNum);
			break;
		}
		_vid.updateDisplay(1, _stub);
		_stub->sleep(50);
	}
	_state = kStateGame;
}

void Engine::saveGameState(uint8 slot, const char *desc) {
	warning("STUB: Engine::saveGameState()");
}

void Engine::loadGameState(uint8 slot) {
	warning("STUB: Engine::loadGameState()");
}

} // namespace Awe
