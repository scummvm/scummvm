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

#include "xeen/dialogs/dialogs_input.h"
#include "xeen/scripts.h"
#include "xeen/xeen.h"

namespace Xeen {

int Input::show(XeenEngine *vm, Window *window, Common::String &line,
		uint maxLen, int maxWidth, bool isNumeric) {
	Input *dlg = new Input(vm, window);
	int result = dlg->getString(line, maxLen, maxWidth, isNumeric);
	delete dlg;

	return result;
}

int Input::nonEnToLower(uint16 ascii) {
	if (Common::RU_RUS == g_vm->getLanguage()) {
		switch (ascii) {
		case Common::KEYCODE_f:            return 0xA0;  // А
		case Common::KEYCODE_COMMA:        return 0xA1;  // Б
		case Common::KEYCODE_d:            return 0xA2;  // В
		case Common::KEYCODE_u:            return 0xA3;  // Г
		case Common::KEYCODE_l:            return 0xA4;  // Д
		case Common::KEYCODE_t:            return 0xA5;  // Е
		case Common::KEYCODE_BACKQUOTE:    return 0xF1;  // Ё
		case Common::KEYCODE_SEMICOLON:    return 0xA6;  // Ж
		case Common::KEYCODE_p:            return 0xA7;  // З
		case Common::KEYCODE_b:            return 0xA8;  // И
		case Common::KEYCODE_q:            return 0xA9;  // Й
		case Common::KEYCODE_r:            return 0xAA;  // К
		case Common::KEYCODE_k:            return 0xAB;  // Л
		case Common::KEYCODE_v:            return 0xAC;  // М
		case Common::KEYCODE_y:            return 0xAD;  // Н
		case Common::KEYCODE_j:            return 0xAE;  // О
		case Common::KEYCODE_g:            return 0xAF;  // П
		case Common::KEYCODE_h:            return 0xE0;  // Р
		case Common::KEYCODE_c:            return 0xE1;  // С
		case Common::KEYCODE_n:            return 0xE2;  // Т
		case Common::KEYCODE_e:            return 0xE3;  // У
		case Common::KEYCODE_a:            return 0xE4;  // Ф
		case Common::KEYCODE_LEFTBRACKET:  return 0xE5;  // Х
		case Common::KEYCODE_w:            return 0xE6;  // Ц
		case Common::KEYCODE_x:            return 0xE7;  // Ч
		case Common::KEYCODE_i:            return 0xE8;  // Ш
		case Common::KEYCODE_o:            return 0xE9;  // Щ
		case Common::KEYCODE_RIGHTBRACKET: return 0xEA;  // Ъ
		case Common::KEYCODE_s:            return 0xEB;  // Ы
		case Common::KEYCODE_m:            return 0xEC;  // Ь
		case Common::KEYCODE_QUOTE:        return 0xED;  // Э
		case Common::KEYCODE_PERIOD:       return 0xEE;  // Ю
		case Common::KEYCODE_z:            return 0xEF;  // Я
		default:
			return tolower(ascii);
		}
	}
	return ascii;
}

int Input::getString(Common::String &line, uint maxLen, int maxWidth, bool isNumeric) {
	bool nonEnCharset = false;

	_vm->_noDirectionSense = true;
	Common::String msg = Common::String::format("\x3""l\t000\x4%03d\x3""c", maxWidth);
	_window->writeString(msg);
	_window->update();

	while (!_vm->shouldExit()) {
		Common::KeyState keyState = waitForKey(msg);
		const Common::KeyCode keyCode = keyState.keycode;

		bool refresh = false;
		if ((keyCode == Common::KEYCODE_BACKSPACE || keyCode == Common::KEYCODE_DELETE)
				&& line.size() > 0) {
			line.deleteLastChar();
			refresh = true;
		} else if (line.size() < maxLen && (line.size() > 0 || keyCode != Common::KEYCODE_SPACE)
				&& ((isNumeric && keyState.ascii >= '0' && keyState.ascii <= '9') ||
				   (!isNumeric && keyState.ascii >= ' ' && keyState.ascii <= (char)127))) {
			if (!nonEnCharset) {
				if (!isNumeric && Common::isAlpha(keyState.ascii)) {
					// The original game doesn't care about Shift or Caps Locks. The
					// capitalization is done for the user automatically at the beginning of
					// words.
					if (line.empty() || line.hasSuffix(" ")) {
						line += toupper(keyState.ascii);
					} else {
						line += tolower(keyState.ascii);
					}
				} else {
					line += keyState.ascii;
				}
			} else {
				if (!isNumeric) {
					line += nonEnToLower(keyState.ascii);
				} else {
					line += keyState.ascii;
				}
			}

			refresh = true;
		} else if (keyCode == Common::KEYCODE_RETURN || keyCode == Common::KEYCODE_KP_ENTER) {
			break;
		} else if (keyCode == Common::KEYCODE_ESCAPE) {
			line = "";
			break;
		} else if (Common::RU_RUS == g_vm->getLanguage()) {
			if (Common::KEYCODE_F11 == keyCode) {
				nonEnCharset = true;
			} else if (Common::KEYCODE_F12 == keyCode) {
				nonEnCharset = false;
			}
		}

		if (refresh) {
			msg = Common::String::format("\x3""l\t000\x4%03d\x3""c%s", maxWidth, line.c_str());
			_window->writeString(msg);
			_window->update();
		}
	}

	_vm->_noDirectionSense = false;
	return line.size();
}

Common::KeyState Input::waitForKey(const Common::String &msg) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Windows &windows = *_vm->_windows;

	bool oldUpDoorText = intf._upDoorText;
	byte oldTillMove = intf._tillMove;
	intf._upDoorText = false;
	intf._tillMove = 0;

	bool flag = !_vm->_startupWindowActive && !windows[25]._enabled
		&& _vm->_mode != MODE_FF && _vm->_mode != MODE_INTERACTIVE7;

	PendingEvent pe;
	while (!_vm->shouldExit()) {
		events.updateGameCounter();

		if (flag)
			intf.draw3d(false);
		_window->writeString(msg);
		animateCursor();
		_window->update();

		if (flag)
			windows[3].update();

		events.wait(1);
		if (events.getEvent(pe) && pe.isKeyboard())
			break;
	}

	_window->writeString("");
	_window->update();

	intf._tillMove = oldTillMove;
	intf._upDoorText = oldUpDoorText;

	return pe._keyState;
}

void Input::animateCursor() {
	// Iterate through each frame
	_cursorAnimIndex = _cursorAnimIndex ? _cursorAnimIndex - 1 : 5;
	static const char CURSOR_ANIMATION_IDS[] = { 32, 124, 126, 127, 126, 124 };

	// Form a string for the cursor and write it out
	Common::Point writePos = _window->_writePos;
	_window->writeCharacter(CURSOR_ANIMATION_IDS[_cursorAnimIndex]);
	_window->_writePos = writePos;
}

/*------------------------------------------------------------------------*/

StringInput::StringInput(XeenEngine *vm): Input(vm, &(*vm->_windows)[6]) {
}

int StringInput::show(XeenEngine *vm, bool type, const Common::String &msg1,
		const Common::String &msg2, int opcode) {
	StringInput *dlg = new StringInput(vm);
	int result = dlg->execute(type, msg1, msg2, opcode);
	delete dlg;

	return result;
}

int StringInput::execute(bool type, const Common::String &expected,
		const Common::String &title, int opcode) {
	FileManager &files = *_vm->_files;
	Scripts &scripts = *_vm->_scripts;
	Windows &windows = *_vm->_windows;
	Window &w = windows[6];
	Sound &sound = *_vm->_sound;
	int result = 0;

	w.open();
	w.writeString(Common::String::format("\r\x03""c%s\v024\t000", title.c_str()));
	w.update();

	Common::String line;
	if (getString(line, 30, 200, false)) {
		if (type) {
			// WORKAROUND: Fix for incorrect answer for one of the Vowelless Knight riddles
			line.toLowercase();
			if (line == "iieeeoeeeouie")
				line = "iieeeoeeaouie";
			Common::String scriptsMsg = scripts._message;
			scriptsMsg.toLowercase();
			if (scriptsMsg == "iieeeoeeeouie")
				scriptsMsg = "iieeeoeeaouie";

			if (line == scriptsMsg) {
				result = true;
			} else if (!line.compareToIgnoreCase(expected)) {
				result = (opcode == 55) ? -1 : 1;
			}
		} else {
			// Load in the mirror list
			MirrorEntry me;
			scripts._mirror.clear();

			File f(Common::String::format("%smirr.txt", files._ccNum ? "dark" : "xeen"), 1);
			while (me.synchronize(f))
				scripts._mirror.push_back(me);
			f.close();

			// Load in any extended mirror entries
			Common::File f2;
			if (f2.open(Common::String::format("%smirr.ext", files._ccNum ? "dark" : "xeen"))) {
				while (me.synchronize(f2))
					scripts._mirror.push_back(me);
				f2.close();
			}

			if (Common::RU_RUS == g_vm->getLanguage() && GType_Clouds == g_vm->getGameID()) {
				for (uint idx = 0; idx < 59; ++idx) {
					if (!line.compareToIgnoreCase(Res.CLOUDS_MIRROR_LOCATIONS[idx])) {
						result = idx + 1;
						sound.playFX(_vm->_files->_ccNum ? 35 : 61);
						break;
					}
				}
			} else {
				for (uint idx = 0; idx < scripts._mirror.size(); ++idx) {
					if (!line.compareToIgnoreCase(scripts._mirror[idx]._name)) {
						result = idx + 1;
						sound.playFX(_vm->_files->_ccNum ? 35 : 61);
						break;
					}
				}
			}
		}
	}

	w.close();
	return result;
}

/*------------------------------------------------------------------------*/

NumericInput::NumericInput(XeenEngine *vm, int window) : Input(vm, &(*vm->_windows)[window]) {
}

int NumericInput::show(XeenEngine *vm, int window, int maxLength, int maxWidth) {
	NumericInput *dlg = new NumericInput(vm, window);
	int result = dlg->execute(maxLength, maxWidth);
	delete dlg;

	return result;
}

int NumericInput::execute(int maxLength, int maxWidth) {
	Common::String line;

	if (getString(line, maxLength, maxWidth, true))
		return atoi(line.c_str());
	else
		return 0;
}

/*------------------------------------------------------------------------*/

int Choose123::show(XeenEngine *vm, uint numOptions) {
	assert(numOptions <= 9);
	Choose123 *dlg = new Choose123(vm);
	int result = dlg->execute(numOptions);
	delete dlg;

	return result;
}

int Choose123::execute(uint numOptions) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	LocationManager &loc = *_vm->_locations;
	Windows &windows = *_vm->_windows;
	Resources &res = *_vm->_resources;

	Mode oldMode = _vm->_mode;
	_vm->_mode = MODE_DIALOG_123;

	loadButtons(numOptions);
	res._globalSprites.draw(0, 7, Common::Point(232, 74));
	drawButtons(&windows[0]);
	windows[34].update();

	int result = -1;
	while (result == -1) {
		do {
			events.updateGameCounter();
			int delay;
			if (loc.isActive()) {
				loc.drawAnim(true);
				delay = 3;
			} else {
				intf.draw3d(true);
				delay = 1;
			}

			events.wait(delay);
			checkEvents(_vm);

			if (_vm->shouldExit())
				return 0;
		} while (!_buttonValue);

		if (_buttonValue == Common::KEYCODE_ESCAPE) {
			result = 0;
		} else if (_buttonValue >= Common::KEYCODE_1 && _buttonValue < (Common::KEYCODE_1 + (int)numOptions)) {
			_buttonValue -= Common::KEYCODE_0;
			result = (_buttonValue == (int)numOptions) ? 0 : _buttonValue;
		}
	}

	_vm->_mode = oldMode;
	intf.mainIconsPrint();

	return result;
}

void Choose123::loadButtons(uint numOptions) {
	assert(numOptions > 0 && numOptions <= 9);
	_iconSprites.load("choose.icn");
	const int XPOS[3] = { 235, 260, 286 };
	const int YPOS[3] = { 75, 96, 117 };

	for (uint idx = 0; idx < numOptions; ++idx) {
		Common::Rect r(24, 20);
		r.moveTo(XPOS[idx % 3], YPOS[idx / 3]);
		addButton(r, Common::KEYCODE_1 + idx, &_iconSprites);
	}
}

/*------------------------------------------------------------------------*/

int HowMuch::show(XeenEngine *vm) {
	HowMuch *dlg = new HowMuch(vm);
	int result = dlg->execute();
	delete dlg;

	return result;
}

int HowMuch::execute() {
	Windows &windows = *_vm->_windows;
	Window &w = windows[6];
	Common::String num;

	w.open();
	w.writeString(Res.HOW_MUCH);
	w.update();
	int lineSize = Input::show(_vm, &w, num, 8, 70, true);
	w.close();

	if (!lineSize)
		return -1;
	return atoi(num.c_str());
}

} // End of namespace Xeen
