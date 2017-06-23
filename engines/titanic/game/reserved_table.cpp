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

#include "titanic/game/reserved_table.h"
#include "titanic/core/room_item.h"
#include "titanic/core/view_item.h"
#include "titanic/npcs/maitre_d.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CReservedTable, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(PlayerTriesRestaurantTableMsg)
END_MESSAGE_MAP()

void CReservedTable::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	file->writeNumberLine(_tableId, indent);

	CGameObject::save(file, indent);
}

void CReservedTable::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	_tableId = file->readNumber();
	CGameObject::load(file);
}

bool CReservedTable::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_flag) {
		CPlayerTriesRestaurantTableMsg tryMsg(_tableId, 0);
		tryMsg.execute(findRoom(), CReservedTable::_type, MSGFLAG_CLASS_DEF | MSGFLAG_SCAN);
	}

	return true;
}

bool CReservedTable::PlayerTriesRestaurantTableMsg(CPlayerTriesRestaurantTableMsg *msg) {
	if (msg->_tableId == _tableId) {
		if (!msg->_result) {
			CMaitreD *maitreD = dynamic_cast<CMaitreD *>(findRoomObject("MaitreD"));
			startTalking(maitreD, 118, maitreD->findView());
			msg->_result = true;
		}

		_cursorId = CURSOR_INVALID;
		_flag = true;
		return true;
	} else {
		return false;
	}
}

} // End of namespace Titanic
