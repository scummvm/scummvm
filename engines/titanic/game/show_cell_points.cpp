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

#include "titanic/game/show_cell_points.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CShowCellpoints, CGameObject)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

void CShowCellpoints::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_npcName, indent);
	file->writeNumberLine(_flag, indent);
	CGameObject::save(file, indent);
}

void CShowCellpoints::load(SimpleFile *file) {
	file->readNumber();
	_npcName = file->readString();
	_flag = file->readNumber();
	CGameObject::load(file);
}

bool CShowCellpoints::EnterViewMsg(CEnterViewMsg *msg) {
	CPetControl *pet = getPetControl();
	if (pet) {
		petSetArea(PET_CONVERSATION);
		pet->setActiveNPC(_npcName);
		pet->incAreaLocks();
		_flag = true;
	}

	return true;
}

bool CShowCellpoints::LeaveViewMsg(CLeaveViewMsg *msg) {
	CPetControl *pet = getPetControl();
	if (pet && _flag) {
		pet->resetDials0();
		pet->decAreaLocks();
		_flag = false;
	}

	return true;
}

} // End of namespace Titanic
