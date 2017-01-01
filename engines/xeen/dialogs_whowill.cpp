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

#include "xeen/dialogs_whowill.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

int WhoWill::show(XeenEngine *vm, int message, int action, bool type) {
	WhoWill *dlg = new WhoWill(vm);
	int result = dlg->execute(message, action, type);
	delete dlg;

	return result;
}

int WhoWill::execute(int message, int action, bool type) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Scripts &scripts = *_vm->_scripts;
	Town &town = *_vm->_town;
	int numFrames;

	if (party._activeParty.size() <= 1)
		// Unless there's at least two characters, just return the first one
		return 1;

	screen._windows[38].close();
	screen._windows[12].close();

	Common::String actionStr = type ? map._events._text[action] : Res.WHO_WILL_ACTIONS[action];
	Common::String msg = Common::String::format(Res.WHO_WILL, actionStr.c_str(),
		Res.WHO_ACTIONS[message], party._activeParty.size());

	screen._windows[36].open();
	screen._windows[36].writeString(msg);
	screen._windows[36].update();

	intf._face1State = map._headData[party._mazePosition.y][party._mazePosition.x]._left;
	intf._face2State = map._headData[party._mazePosition.y][party._mazePosition.x]._right;

	while (!_vm->shouldQuit()) {
		events.updateGameCounter();

		if (screen._windows[11]._enabled) {
			town.drawTownAnim(0);
			screen._windows[36].frame();
			numFrames = 3;
		} else {
			intf.draw3d(false);
			screen._windows[36].frame();
			screen._windows[3].update();
			numFrames = 1;
		}

		events.wait(numFrames);
		checkEvents(_vm);
		if (!_buttonValue)
			continue;

		if (_buttonValue == 27) {
			_buttonValue = 0;
			break;
		} else if (_buttonValue >= Common::KEYCODE_F1 && _buttonValue <= Common::KEYCODE_F6) {
			_buttonValue -= Common::KEYCODE_F1 - 1;
			if (_buttonValue > (int)party._activeParty.size())
				continue;

			if (party._activeParty[_buttonValue - 1].noActions())
				continue;

			scripts._whoWill = _buttonValue;
			break;
		}
	}

	intf._face1State = intf._face2State = 2;
	screen._windows[36].close();
	return _buttonValue;
}

} // End of namespace Xeen
