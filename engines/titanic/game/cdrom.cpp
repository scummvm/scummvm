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

#include "titanic/game/cdrom.h"
#include "titanic/core/room_item.h"
#include "titanic/game/cdrom_tray.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCDROM, CGameObject)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(MouseDragEndMsg)
	ON_MESSAGE(MouseDragMoveMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

CCDROM::CCDROM() : CGameObject() {
}

void CCDROM::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writePoint(_centroid, indent);
	CGameObject::save(file, indent);
}

void CCDROM::load(SimpleFile *file) {
	file->readNumber();
	_centroid = file->readPoint();
	CGameObject::load(file);
}

bool CCDROM::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (checkStartDragging(msg)) {
		hideMouse();
		_centroid = msg->_mousePos - _bounds;
		setPosition(msg->_mousePos - _centroid);
		return true;
	} else {
		return false;
	}
}

bool CCDROM::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	showMouse();

	if (msg->_dropTarget && msg->_dropTarget->getName() == "newComputer") {
		CCDROMTray *newTray = dynamic_cast<CCDROMTray *>(getRoom()->findByName("newTray"));

		if (newTray->_isOpened && newTray->_insertedCD == "None") {
			CActMsg actMsg(getName());
			actMsg.execute(newTray);
			setVisible(false);
		}
	}

	resetPosition();
	return true;
}

bool CCDROM::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	setPosition(msg->_mousePos - _centroid);
	return true;
}

bool CCDROM::ActMsg(CActMsg *msg) {
	if (msg->_action == "Ejected")
		setVisible(true);

	return true;
}

} // End of namespace Titanic
