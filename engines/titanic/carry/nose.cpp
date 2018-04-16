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

#include "titanic/carry/nose.h"
#include "titanic/game/head_slot.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CNose, CHeadPiece)
	ON_MESSAGE(ChangeSeasonMsg)
	ON_MESSAGE(UseWithOtherMsg)
END_MESSAGE_MAP()

CNose::CNose() : CHeadPiece() {
}

void CNose::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CHeadPiece::save(file, indent);
}

void CNose::load(SimpleFile *file) {
	file->readNumber();
	CHeadPiece::load(file);
}

bool CNose::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	// WORKAROUND: Redundant code in original skipped
	return true;
}

bool CNose::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CHeadSlot *slot = dynamic_cast<CHeadSlot *>(msg->_other);
	if (!slot)
		return CCarry::UseWithOtherMsg(msg);

	petMoveToHiddenRoom();
	_flag = false;
	CAddHeadPieceMsg addMsg(getName());
	if (addMsg._value != "NULL")
		addMsg.execute("NoseSlot");

	return true;
}

} // End of namespace Titanic
