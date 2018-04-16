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

#include "titanic/carry/hammer.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CHammer, CCarry)
	ON_MESSAGE(PuzzleSolvedMsg)
	ON_MESSAGE(UseWithOtherMsg)
END_MESSAGE_MAP()

CHammer::CHammer() : CCarry() {
}

void CHammer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CCarry::save(file, indent);
}

void CHammer::load(SimpleFile *file) {
	file->readNumber();
	CCarry::load(file);
}

bool CHammer::PuzzleSolvedMsg(CPuzzleSolvedMsg *msg) {
	_canTake = true;
	return true;
}

bool CHammer::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CString name = msg->_other->getName();
	if (name == "LongStickDispenser") {
		CPuzzleSolvedMsg solvedMsg;
		solvedMsg.execute("LongStickDispenser");
	} else if (name == "Bomb") {
		CActMsg actMsg("Hit");
		actMsg.execute("Bomb");
	}

	return CCarry::UseWithOtherMsg(msg);
}

} // End of namespace Titanic
