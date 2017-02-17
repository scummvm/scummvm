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

#include "titanic/game/play_on_act.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPlayOnAct, CBackground)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

void CPlayOnAct::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CBackground::save(file, indent);
}

void CPlayOnAct::load(SimpleFile *file) {
	file->readNumber();
	CBackground::load(file);
}

bool CPlayOnAct::ActMsg(CActMsg *msg) {
	if (msg->_action == "PlayMovie") {
		setVisible(true);
		playMovie(0);
	} else if (msg->_action == "PlayToEnd") {
		setVisible(true);
		playMovie(MOVIE_WAIT_FOR_FINISH);
	}

	return true;
}

bool CPlayOnAct::LeaveViewMsg(CLeaveViewMsg *msg) {
	return true;
}

} // End of namespace Titanic
