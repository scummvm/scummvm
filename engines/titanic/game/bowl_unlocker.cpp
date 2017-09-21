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

#include "titanic/game/bowl_unlocker.h"
#include "titanic/core/room_item.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBowlUnlocker, CGameObject)
	ON_MESSAGE(NutPuzzleMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

void CBowlUnlocker::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_bowlUnlocked, indent);
	CGameObject::save(file, indent);
}

void CBowlUnlocker::load(SimpleFile *file) {
	file->readNumber();
	_bowlUnlocked = file->readNumber();
	CGameObject::load(file);
}

bool CBowlUnlocker::NutPuzzleMsg(CNutPuzzleMsg *msg) {
	if (msg->_action == "UnlockBowl") {
		setVisible(true);
		playMovie(MOVIE_NOTIFY_OBJECT);
	}

	return true;
}

bool CBowlUnlocker::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(false);
	_bowlUnlocked = true;

	CNutPuzzleMsg puzzleMsg("BowlUnlocked");
	puzzleMsg.execute(getRoom(), nullptr, MSGFLAG_SCAN);

	playSound(TRANSLATE("z#47.wav", "z#578.wav"));
	return true;
}

bool CBowlUnlocker::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_bowlUnlocked)
		msg->execute("Ear1");
	return true;
}

bool CBowlUnlocker::LeaveViewMsg(CLeaveViewMsg *msg) {
	_bowlUnlocked = false;
	return true;
}

} // End of namespace Titanic
