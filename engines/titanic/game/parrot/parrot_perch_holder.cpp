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

#include "titanic/game/parrot/parrot_perch_holder.h"
#include "titanic/game/cage.h"
#include "titanic/core/project_item.h"
#include "titanic/npcs/parrot.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CParrotPerchHolder, CMultiDropTarget)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(DropObjectMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

void CParrotPerchHolder::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CMultiDropTarget::save(file, indent);
}

void CParrotPerchHolder::load(SimpleFile *file) {
	file->readNumber();
	CMultiDropTarget::load(file);
}

bool CParrotPerchHolder::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!CParrot::_eatingChicken) {
		if (CCage::_open) {
			petDisplayMessage(CANNOT_TAKE_CAGE_LOCKED);
		} else if (CParrot::_state == PARROT_IN_CAGE) {
			CTrueTalkTriggerActionMsg triggerMsg(280252, 0, 0);
			triggerMsg.execute(getRoot(), CParrot::_type,
				MSGFLAG_CLASS_DEF | MSGFLAG_BREAK_IF_HANDLED | MSGFLAG_SCAN);
		}
	}

	return true;
}

bool CParrotPerchHolder::StatusChangeMsg(CStatusChangeMsg *msg) {
	_dropEnabled = msg->_newStatus;
	return true;
}

bool CParrotPerchHolder::DropObjectMsg(CDropObjectMsg *msg) {
	if (CCage::_open)
		return false;
	else
		return CMultiDropTarget::DropObjectMsg(msg);
}

bool CParrotPerchHolder::ActMsg(CActMsg *msg) {
	if (msg->_action == "FlashCore") {
		playMovie(2, 2, 0);
		playMovie(1, 1, 0);
	}

	return true;
}

} // End of namespace Titanic
