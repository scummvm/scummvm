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

#include "titanic/carry/crushed_tv.h"
#include "titanic/npcs/character.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCrushedTV, CCarry)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(UseWithCharMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

CCrushedTV::CCrushedTV() : CCarry() {
}

void CCrushedTV::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CCarry::save(file, indent);
}

void CCrushedTV::load(SimpleFile *file) {
	file->readNumber();
	CCarry::load(file);
}

bool CCrushedTV::ActMsg(CActMsg *msg) {
	if (msg->_action == "SmashTV") {
		setVisible(true);
		_canTake = true;
	}

	return true;
}

bool CCrushedTV::UseWithCharMsg(CUseWithCharMsg *msg) {
	if (msg->_character->getName() == "Barbot" && msg->_character->_visible) {
		setVisible(false);
		CActMsg actMsg("CrushedTV");
		actMsg.execute(msg->_character);
		return true;
	} else {
		return CCarry::UseWithCharMsg(msg);
	}
}

bool CCrushedTV::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!checkStartDragging(msg)) {
		return false;
	} else if (compareViewNameTo("BottomOfWell.Node 7.N")) {
		changeView("BottomOfWell.Node 12.N", "");
		CActMsg actMsg("TelevisionTaken");
		actMsg.execute("BOWTelevisionMonitor");
	}

	return CCarry::MouseDragStartMsg(msg);
}

} // End of namespace Titanic
