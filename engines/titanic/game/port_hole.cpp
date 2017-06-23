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

#include "titanic/game/port_hole.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPortHole, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

CPortHole::CPortHole() : CGameObject(), _open(false),
		_closeSoundName("b#47.wav"), _openSoundName("b#46.wav") {
}

void CPortHole::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_open, indent);
	file->writeQuotedLine(_closeSoundName, indent);
	file->writeQuotedLine(_openSoundName, indent);

	CGameObject::save(file, indent);
}

void CPortHole::load(SimpleFile *file) {
	file->readNumber();
	_open = file->readNumber();
	_closeSoundName = file->readString();
	_openSoundName = file->readString();

	CGameObject::load(file);
}

bool CPortHole::ActMsg(CActMsg *msg) {
	if (msg->_action == "TogglePortHole") {
		if (_open) {
			playMovie(14, 26, MOVIE_NOTIFY_OBJECT);
			playSound(_closeSoundName);
			_open = false;
		} else {
			setVisible(true);
			playMovie(1, 13, 0);
			playSound(_openSoundName);
			_open = true;
		}
	}

	return true;
}

bool CPortHole::MovieEndMsg(CMovieEndMsg *msg) {
	_open = false;
	setVisible(false);
	return true;
}

bool CPortHole::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_open) {
		playSound(_closeSoundName);
		playMovie(14, 26, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		_open = false;
	}

	return true;
}

bool CPortHole::EnterViewMsg(CEnterViewMsg *msg) {
	setVisible(false);
	_open = false;
	return true;
}

} // End of namespace Titanic
