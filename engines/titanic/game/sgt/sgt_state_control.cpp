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

#include "titanic/game/sgt/sgt_state_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSGTStateControl, CBackground)
	ON_MESSAGE(PETUpMsg)
	ON_MESSAGE(PETDownMsg)
	ON_MESSAGE(PETActivateMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

void CSGTStateControl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_state, indent);
	CBackground::save(file, indent);
}

void CSGTStateControl::load(SimpleFile *file) {
	file->readNumber();
	_state = file->readNumber();
	CBackground::load(file);
}

bool CSGTStateControl::PETUpMsg(CPETUpMsg *msg) {
	// WORKAROUND: Redundant code in original not included
	return true;
}

bool CSGTStateControl::PETDownMsg(CPETDownMsg *msg) {
	// WORKAROUND: Redundant code in original not included
	return true;
}

bool CSGTStateControl::PETActivateMsg(CPETActivateMsg *msg) {
	if (msg->_name == "SGTSelector") {
		static const char *const TARGETS[] = {
			"Vase", "Bedfoot", "Toilet", "Drawer", "SGTTV", "Armchair", "BedHead",
			"WashStand", "Desk", "DeskChair", "Basin", "ChestOfDrawers"
		};
		_state = msg->_numValue;
		CActMsg actMsg;
		actMsg.execute(TARGETS[_state]);
	}

	return true;
}

bool CSGTStateControl::EnterViewMsg(CEnterViewMsg *msg) {
	_state = 1;
	petSetRemoteTarget();
	return true;
}

bool CSGTStateControl::LeaveViewMsg(CLeaveViewMsg *msg) {
	petClear();
	return true;
}

} // End of namespace Titanic
