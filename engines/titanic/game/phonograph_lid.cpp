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

#include "titanic/game/phonograph_lid.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPhonographLid, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(LockPhonographMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

void CPhonographLid::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_open, indent);
	CGameObject::save(file, indent);
}

void CPhonographLid::load(SimpleFile *file) {
	file->readNumber();
	_open = file->readNumber();
	CGameObject::load(file);
}

bool CPhonographLid::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CQueryPhonographState stateMsg;
	stateMsg.execute(getParent(), nullptr, MSGFLAG_SCAN);
	if (stateMsg._value) {
		petDisplayMessage(0, LOCKED_MUSIC_SYSTEM);
	} else {
		if (_open) {
			playMovie(27, 55, 0);
		} else {
			CGameObject *lock = dynamic_cast<CGameObject *>(findByName("Music System Lock"));
			if (lock)
				lock->setVisible(false);
			playMovie(0, 27, 0);
		}

		_open = !_open;
	}

	return true;
}

bool CPhonographLid::MovieEndMsg(CMovieEndMsg *msg) {
	// WORKAROUND: Redundant code in original not included
	return true;
}

bool CPhonographLid::LockPhonographMsg(CLockPhonographMsg *msg) {
	_cursorId = msg->_value ? CURSOR_INVALID : CURSOR_ARROW;
	return true;
}

bool CPhonographLid::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_open) {
		playMovie(27, 55, MOVIE_WAIT_FOR_FINISH);
		_open = false;
	}

	return true;
}

} // End of namespace Titanic
