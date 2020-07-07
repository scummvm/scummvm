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

int Input::getString(Common::String &line, uint maxLen, int maxWidth, bool isNumeric) {
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

			refresh = true;
		} else if (keyCode == Common::KEYCODE_RETURN || keyCode == Common::KEYCODE_KP_ENTER) {
			break;
		} else if (keyCode == Common::KEYCODE_ESCAPE) {
			line = "";
			break;
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

			for (uint idx = 0; idx < scripts._mirror.size(); ++idx) {
				if (!line.compareToIgnoreCase(scripts._mirror[idx]._name)) {
					result = idx + 1;
					sound.playFX(_vm->_files->_ccNum ? 35 : 61);
					break;
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
