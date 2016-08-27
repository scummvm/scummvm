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

#include "titanic/game/pickup/pick_up_bar_glass.h"
#include "titanic/core/project_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPickUpBarGlass, CPickUp)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

void CPickUpBarGlass::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CPickUp::save(file, indent);
}

void CPickUpBarGlass::load(SimpleFile *file) {
	file->readNumber();
	CPickUp::load(file);
}

bool CPickUpBarGlass::StatusChangeMsg(CStatusChangeMsg *msg) {
	switch (msg->_newStatus) {
	case 0:
		setVisible(false);
		_enabled = false;
		break;
	case 1:
		setVisible(true);
		_enabled = true;
		break;
	case 2:
		setVisible(true);
		_enabled = false;
		break;
	default:
		break;
	}

	return true;
}

bool CPickUpBarGlass::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (checkStartDragging(msg) && _enabled) {
		CTurnOn onMsg;
		onMsg.execute("BeerGlass");
		CVisibleMsg visibleMsg;
		visibleMsg.execute("BeerGlass");
		CPassOnDragStartMsg passMsg(msg->_mousePos, 1, 3);
		passMsg.execute("BeerGlass");

		msg->_dragItem = getRoot()->findByName("BeerGlass");

		CActMsg actMsg("PlayerTakesGlass");
		actMsg.execute("Barbot");
		return true;
	} else {
		return false;
	}
}

bool CPickUpBarGlass::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return true;
}

} // End of namespace Titanic
