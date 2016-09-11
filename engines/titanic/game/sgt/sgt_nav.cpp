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

#include "titanic/game/sgt/sgt_nav.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(SGTNav, CSGTStateRoom)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseMoveMsg)
END_MESSAGE_MAP()

void SGTNav::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CSGTStateRoom::save(file, indent);
}

void SGTNav::load(SimpleFile *file) {
	file->readNumber();
	CSGTStateRoom::load(file);
}

bool SGTNav::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CTurnOn onMsg;
	CTurnOff offMsg;

	if (_statics->_v6 == "Open" && _statics->_v1 == "Open") {
		if (_statics->_v3 == "Open")
			offMsg.execute("Vase");
		if (_statics->_v4 == "Closed")
			onMsg.execute("SGTTV");
		if (_statics->_v7 == "Open")
			offMsg.execute("Drawer");
		if (_statics->_v8 == "Open")
			offMsg.execute("Armchair");
		if (_statics->_v9 == "Open")
			offMsg.execute("Deskchair");
		if (_statics->_v12 == "Open")
			offMsg.execute("Toilet");

		changeView("SGTState.Node 2.E");
	} else if (_statics->_v1 == "Open") {
		petDisplayMessage(1, YOUR_STATEROOM);
	} else if (_statics->_v6 == "Closed") {
		petDisplayMessage(1, BED_NOT_SUPPORT_YOUR_WEIGHT);
	}

	return true;
}

bool SGTNav::MouseMoveMsg(CMouseMoveMsg *msg) {
	if (_statics->_v6 == "Open" && _statics->_v1 == "Open")
		_cursorId = CURSOR_MOVE_FORWARD;
	else
		_cursorId = CURSOR_ARROW;

	return true;
}

} // End of namespace Titanic
