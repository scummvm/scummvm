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

#include "titanic/game/pickup/pick_up_lemon.h"
#include "titanic/core/project_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPickUpLemon, CPickUp)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

void CPickUpLemon::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CPickUp::save(file, indent);
}

void CPickUpLemon::load(SimpleFile *file) {
	file->readNumber();
	CPickUp::load(file);
}

bool CPickUpLemon::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return true;
}

bool CPickUpLemon::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!checkStartDragging(msg))
		return true;
	if (!_enabled)
		return false;

	CVisibleMsg visibleMsg;
	visibleMsg.execute("Lemon");
	CPassOnDragStartMsg passMsg(msg->_mousePos, 1);
	passMsg.execute("Lemon");

	msg->_dragItem = getRoot()->findByName("Lemon");
	return true;
}

} // End of namespace Titanic
