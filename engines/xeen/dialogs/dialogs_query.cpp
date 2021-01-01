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

#include "xeen/dialogs/dialogs_query.h"
#include "xeen/xeen.h"

namespace Xeen {

bool Confirm::show(XeenEngine *vm, const Common::String &msg, int mode) {
	Confirm *dlg = new Confirm(vm);
	bool result = dlg->execute(msg, mode);
	delete dlg;

	return result;
}

bool Confirm::execute(const Common::String &msg, int mode) {
	EventsManager &events = *_vm->_events;
	Windows &windows = *_vm->_windows;
	SpriteResource confirmSprites;

	confirmSprites.load("confirm.icn");
	addButton(Common::Rect(129, 112, 153, 122), Common::KEYCODE_y, &confirmSprites);
	addButton(Common::Rect(185, 112, 209, 122), Common::KEYCODE_n, &confirmSprites);

	Window &w = windows[mode ? 22 : 21];
	w.open();

	if (!mode) {
		confirmSprites.draw(w, 0, Common::Point(129, 112));
		confirmSprites.draw(w, 2, Common::Point(185, 112));
		_buttons[0]._bounds.moveTo(129, 112);
		_buttons[1]._bounds.moveTo(185, 112);
	} else {
		if (mode & 0x80) {
			clearButtons();
		} else {
			confirmSprites.draw(w, 0, Common::Point(120, 133));
			confirmSprites.draw(w, 2, Common::Point(176, 133));
			_buttons[0]._bounds.moveTo(120, 133);
			_buttons[1]._bounds.moveTo(176, 133);
		}
	}

	w.writeString(msg);
	w.update();

	events.clearEvents();
	bool result = false;

	while (!_vm->shouldExit()) {
		_buttonValue = 0;
		while (!_vm->shouldExit() && !_buttonValue) {
			events.pollEvents();
			checkEvents(_vm);
		}

		if ((mode & 0x80) || _buttonValue == Common::KEYCODE_ESCAPE
				|| _buttonValue == Common::KEYCODE_n)
			break;

		if (_buttonValue == Common::KEYCODE_y) {
			result = true;
			break;
		}
	}

	events.clearEvents();
	w.close();
	return result;
}

/*------------------------------------------------------------------------*/

bool YesNo::show(XeenEngine *vm, bool type, bool townFlag) {
	YesNo *dlg = new YesNo(vm);
	bool result = dlg->execute(type, townFlag);
	delete dlg;

	return result;
}

bool YesNo::execute(bool type, bool townFlag) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	LocationManager &loc = *_vm->_locations;
	SpriteResource confirmSprites;
	bool result = false;

	Mode oldMode = _vm->_mode;
	_vm->_mode = oldMode == MODE_7 ? MODE_8 : MODE_7;
	events.clearEvents();
	setWaitBounds();

	if (!type) {
		Map &map = *_vm->_map;
		Party &party = *_vm->_party;
		Resources &res = *_vm->_resources;
		Windows &windows = *_vm->_windows;

		confirmSprites.load("confirm.icn");
		res._globalSprites.draw(0, 7, Common::Point(232, 74));
		confirmSprites.draw(0, 0, Common::Point(235, 75));
		confirmSprites.draw(0, 2, Common::Point(260, 75));
		windows[34].update();

		addButton(Common::Rect(235, 75, 259, 95), Common::KEYCODE_y, &confirmSprites);
		addButton(Common::Rect(260, 75, 284, 95), Common::KEYCODE_n, &confirmSprites);

		intf._face1State = map._headData[party._mazePosition.y][party._mazePosition.x]._left;
		intf._face2State = map._headData[party._mazePosition.y][party._mazePosition.x]._right;
	}

	while (!_vm->shouldExit()) {
		events.updateGameCounter();

		if (loc.isActive()) {
			loc.drawAnim(townFlag);
			//numFrames = 3;
		} else {
			intf.draw3d(true);
			//numFrames = 1;
		}

		events.wait(3);
		checkEvents(_vm);
		if (!_buttonValue)
			continue;

		if (type || _buttonValue == Common::KEYCODE_y) {
			result = true;
			break;
		} else if (_buttonValue == Common::KEYCODE_n || _buttonValue == Common::KEYCODE_ESCAPE)
			break;
	}

	intf._face1State = intf._face2State = 2;
	_vm->_mode = oldMode;

	if (!type)
		intf.mainIconsPrint();

	return result;
}

} // End of namespace Xeen
