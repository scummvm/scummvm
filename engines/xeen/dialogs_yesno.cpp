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

#include "xeen/dialogs_yesno.h"
#include "xeen/xeen.h"

namespace Xeen {

bool YesNo::show(XeenEngine *vm, bool type, int v2) {
	YesNo *dlg = new YesNo(vm);
	bool result = dlg->execute(type, v2);
	delete dlg;

	return result;
}

bool YesNo::execute(bool type, int v2) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	SpriteResource confirmSprites;
	int numFrames;
	bool result = false;

	Mode oldMode = _vm->_mode;
	_vm->_mode = oldMode == MODE_7 ? MODE_8 : MODE_7;

	if (!type) {
		confirmSprites.load("confirm.icn");
		intf._globalSprites.draw(screen, 7, Common::Point(232, 74));
		confirmSprites.draw(screen, 0, Common::Point(235, 75));
		confirmSprites.draw(screen, 2, Common::Point(260, 75));
		screen._windows[34].update();

		addButton(Common::Rect(235, 75, 259, 95), 'Y', &confirmSprites);
		addButton(Common::Rect(260, 75, 284, 95), 'N', &confirmSprites);

		intf._face1State = map._headData[party._mazePosition.y][party._mazePosition.x]._left;
		intf._face2State = map._headData[party._mazePosition.y][party._mazePosition.x]._right;
	}

	while (!_vm->shouldQuit()) {
		events.updateGameCounter();
		
		if (intf._townSprites[0].empty()) {
			intf.draw3d(true);
			numFrames = 1;
		} else {
			intf.drawTownAnim(v2);
			numFrames = 3;
		}

		events.wait(3, true);
		if (!_buttonValue)
			continue;

		if (type || _buttonValue == 'Y') {
			result = true;
			break;
		} else if (_buttonValue == 'N' || _buttonValue == Common::KEYCODE_ESCAPE)
			break;
	}

	intf._face1State = intf._face2State = 2;
	_vm->_mode = oldMode;

	if (!type)
		intf.mainIconsPrint();

	return result;
}

} // End of namespace Xeen
