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

#include "titanic/carry/ear.h"
#include "titanic/game/head_slot.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEar, CHeadPiece)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(UseWithOtherMsg)
END_MESSAGE_MAP()

CEar::CEar() : CHeadPiece() {
}

void CEar::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CHeadPiece::save(file, indent);
}

void CEar::load(SimpleFile *file) {
	file->readNumber();
	CHeadPiece::load(file);
}

bool CEar::ActMsg(CActMsg *msg) {
	if (msg->_action == "MusicSolved")
		_canTake = true;
	return true;
}

bool CEar::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CHeadSlot *slot = dynamic_cast<CHeadSlot *>(msg->_other);
	if (slot) {
		setVisible(false);
		petMoveToHiddenRoom();
		setPosition(Point(0, 0));

		CAddHeadPieceMsg addMsg(getName());
		if (addMsg._value != "NULL")
			addMsg.execute(addMsg._value == "Ear1" ? "Ear1Slot" : "Ear2Slot");

		return true;
	} else {
		return CCarry::UseWithOtherMsg(msg);
	}
}

} // End of namespace Titanic
