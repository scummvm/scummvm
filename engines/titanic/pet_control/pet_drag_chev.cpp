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

#include "titanic/pet_control/pet_drag_chev.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/messages/messages.h"
#include "titanic/npcs/succubus.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPetDragChev, CPetGraphic2)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(MouseDragMoveMsg)
	ON_MESSAGE(MouseDragEndMsg)
END_MESSAGE_MAP()

void CPetDragChev::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CPetGraphic2::save(file, indent);
}

void CPetDragChev::load(SimpleFile *file) {
	file->readNumber();
	CPetGraphic2::load(file);
}

bool CPetDragChev::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	getName();
	return checkStartDragging(msg);
}

bool CPetDragChev::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	dragMove(msg->_mousePos);
	return true;
}

bool CPetDragChev::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	CSuccUBus *succubus = dynamic_cast<CSuccUBus *>(msg->_dropTarget);

	if (succubus) {
		CSetChevRoomBits chevMsg(_destRoomFlags);
		chevMsg.execute(succubus);
		petMoveToHiddenRoom();
	} else {
		CPetControl *petControl = getPetControl();
		if (!petControl || !petControl->contains(msg->_mousePos)
				|| msg->_mousePos.x >= 528 || !petControl->checkDragEnd(this))
			petMoveToHiddenRoom();
	}

	return true;
}

} // End of namespace Titanic
