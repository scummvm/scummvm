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

#include "titanic/game/parrot/parrot_nut_bowl_actor.h"
#include "titanic/core/room_item.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CParrotNutBowlActor, CGameObject)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(BowlStateChangeMsg)
	ON_MESSAGE(IsEarBowlPuzzleDone)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(ReplaceBowlAndNutsMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(NutPuzzleMsg)
END_MESSAGE_MAP()

CParrotNutBowlActor::CParrotNutBowlActor() : CGameObject(),
		_puzzleDone(0), _state(0) {
}

void CParrotNutBowlActor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_puzzleDone, indent);
	file->writeNumberLine(_state, indent);

	CGameObject::save(file, indent);
}

void CParrotNutBowlActor::load(SimpleFile *file) {
	file->readNumber();
	_puzzleDone = file->readNumber();
	_state = file->readNumber();

	CGameObject::load(file);
}

bool CParrotNutBowlActor::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	if (!_state) {
		CActMsg actMsg("Jiggle");
		actMsg.execute("BowlNutsRustler");
	}

	return true;
}

bool CParrotNutBowlActor::BowlStateChangeMsg(CBowlStateChangeMsg *msg) {
	_state = msg->_state;
	if (msg->_state == 3) {
		if (!_puzzleDone) {
			CReplaceBowlAndNutsMsg replaceMsg;
			replaceMsg.execute(findRoom(), nullptr, MSGFLAG_SCAN);
			playSound(TRANSLATE("z#47.wav", "z#578.wav"));
		}

		_puzzleDone = true;
	}

	return true;
}

bool CParrotNutBowlActor::IsEarBowlPuzzleDone(CIsEarBowlPuzzleDone *msg) {
	msg->_value = _puzzleDone;
	return true;
}

bool CParrotNutBowlActor::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return true;
}

bool CParrotNutBowlActor::ReplaceBowlAndNutsMsg(CReplaceBowlAndNutsMsg *msg) {
	if (!_puzzleDone)
		_state = 0;
	return true;
}

bool CParrotNutBowlActor::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (!_puzzleDone && _state) {
		CReplaceBowlAndNutsMsg replaceMsg;
		replaceMsg.execute(findRoom(), nullptr, MSGFLAG_SCAN);
	}

	return true;
}

bool CParrotNutBowlActor::NutPuzzleMsg(CNutPuzzleMsg *msg) {
	if (msg->_action == "NutsGone")
		_state = 1;
	else if (msg->_action == "BowlUnlocked")
		_state = 2;

	return true;
}

} // End of namespace Titanic
