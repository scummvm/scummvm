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

#include "titanic/game/empty_nut_bowl.h"
#include "titanic/core/room_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEmptyNutBowl, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(ReplaceBowlAndNutsMsg)
	ON_MESSAGE(NutPuzzleMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

void CEmptyNutBowl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	CGameObject::save(file, indent);
}

void CEmptyNutBowl::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	CGameObject::load(file);
}

bool CEmptyNutBowl::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_flag) {
		CNutPuzzleMsg nutMsg("UnlockBowl");
		nutMsg.execute(getRoom(), nullptr, MSGFLAG_SCAN);
		_flag = false;
	}

	return true;
}

bool CEmptyNutBowl::ReplaceBowlAndNutsMsg(CReplaceBowlAndNutsMsg *msg) {
	setVisible(false);
	_flag = true;
	return true;
}

bool CEmptyNutBowl::NutPuzzleMsg(CNutPuzzleMsg *msg) {
	if (msg->_action == "NutsGone")
		setVisible(true);
	return true;
}

bool CEmptyNutBowl::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!_flag) {
		msg->execute("Ear1");
		setVisible(false);
	}

	return true;
}

} // End of namespace Titanic
