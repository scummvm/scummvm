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

#include "xeen/dialogs_string_input.h"
#include "xeen/xeen.h"

namespace Xeen {

int StringInput::show(XeenEngine *vm, bool type, const Common::String &msg1,
		const Common::String &msg2, int opcdoe) {
	StringInput *dlg = new StringInput(vm);
	int result = dlg->execute(type, msg1, msg2, opcdoe);
	delete dlg;

	return result;
}

int StringInput::execute(bool type, const Common::String &expected, 
		const Common::String &title, int opcode) {
	Interface &intf = *_vm->_interface;
	Screen &screen = *_vm->_screen;
	Window &w = screen._windows[6];
	SoundManager &sound = *_vm->_sound;
	int result = 0;

	w.open();
	w.writeString(Common::String::format("\r\x03""c%s\v024\t000", title.c_str()));
	w.update();

	Common::String line;
	if (w.getString(line, 30, 200)) {
		if (type) {
			if (line == intf._interfaceText) {
				result = true;
			} else if (line == expected) {
				result = (opcode == 55) ? -1 : 1;
			}
		} else {
			// Load in the mirror list
			File f(Common::String::format("%smirr.txt",
				_vm->_files->_isDarkCc ? "dark" : "xeen"));
			for (int idx = 0; f.pos() < f.size(); ++idx) {
				if (line == f.readLine()) {
					result = idx;
					sound.playFX(_vm->_files->_isDarkCc ? 35 : 61);
					break;
				}
			}
		}
	}

	w.close();
	return result;
}

} // End of namespace Xeen
