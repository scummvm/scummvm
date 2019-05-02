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

#include "titanic/carry/long_stick.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CLongStick, CCarry)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(PuzzleSolvedMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

CLongStick::CLongStick() : CCarry() {
}

void CLongStick::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CCarry::save(file, indent);
}

void CLongStick::load(SimpleFile *file) {
	file->readNumber();
	CCarry::load(file);
}

bool CLongStick::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	if (msg->_other->isEquals("SpeechCentre")) {
		CPuzzleSolvedMsg puzzleMsg;
		puzzleMsg.execute(msg->_other);
	} else if (msg->_other->isEquals("LongStickDispenser")) {
		petDisplayMessage(1, ALREADY_HAVE_STICK);
	} else if (msg->_other->isEquals("Bomb")) {
		CActMsg actMsg("Hit");
		actMsg.execute("Bomb");
	} else {
		return CCarry::UseWithOtherMsg(msg);
	}

	petAddToInventory();
	return true;
}

bool CLongStick::PuzzleSolvedMsg(CPuzzleSolvedMsg *msg) {
	_canTake = true;
	return true;
}

bool CLongStick::LeaveViewMsg(CLeaveViewMsg *msg) {
	setVisible(false);
	return true;
}

} // End of namespace Titanic
