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

#include "titanic/game/starling_puret.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CStarlingPuret, CGameObject)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CStarlingPuret::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	CGameObject::save(file, indent);
}

void CStarlingPuret::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	CGameObject::load(file);
}

bool CStarlingPuret::StatusChangeMsg(CStatusChangeMsg *msg) {
	_flag = msg->_newStatus == 1;
	if (_flag) {
		CStatusChangeMsg changeMsg;
		changeMsg._newStatus = 1;
		changeMsg.execute("StarlingLoop01");
	}

	return true;
}

bool CStarlingPuret::EnterViewMsg(CEnterViewMsg *msg) {
	if (_flag) {
		CStatusChangeMsg changeMsg;
		changeMsg._newStatus = 1;
		changeMsg.execute("PromDeckStarlings");

		playMovie(MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
		CSignalObject signalMsg;
		signalMsg._numValue = 4;
		signalMsg.execute("PromDeckStarlings");
		_flag = false;
	}

	return true;
}

bool CStarlingPuret::MovieEndMsg(CMovieEndMsg *msg) {
	CActMsg actMsg("StarlingsDead");
	actMsg.execute("FanController");
	actMsg.execute("BirdSauceDisp");
	return true;
}

} // End of namespace Titanic
