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

#include "titanic/game/cdrom_computer.h"
#include "titanic/core/room_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCDROMComputer, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

CCDROMComputer::CCDROMComputer() : CGameObject(),
		_clickRect(0, 3, 55, 32) {
}

void CCDROMComputer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_clickRect.left, indent);
	file->writeNumberLine(_clickRect.top, indent);
	file->writeNumberLine(_clickRect.right, indent);
	file->writeNumberLine(_clickRect.bottom, indent);

	CGameObject::save(file, indent);
}

void CCDROMComputer::load(SimpleFile *file) {
	file->readNumber();
	_clickRect.left = file->readNumber();
	_clickRect.top = file->readNumber();
	_clickRect.right = file->readNumber();
	_clickRect.bottom = file->readNumber();

	CGameObject::load(file);
}

bool CCDROMComputer::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CTreeItem *tray = getRoom()->findByName("newTray");
	if (tray) {
		CStatusChangeMsg statusMsg;
		statusMsg.execute(tray);

		if (!statusMsg._success) {
			// Check if the mouse is within the clickable area
			Rect tempRect = _clickRect;
			tempRect.translate(_bounds.left, _bounds.top);

			if (!tempRect.contains(msg->_mousePos))
				return true;
		}

		CActMsg actMsg("ClickedOn");
		actMsg.execute(tray);
	}

	return true;
}

} // End of namespace Titanic
