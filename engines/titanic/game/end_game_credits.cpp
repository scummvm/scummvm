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

#include "titanic/game/end_game_credits.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEndGameCredits, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

CEndGameCredits::CEndGameCredits() : CGameObject(), _flag(false),
	_frameRange(0, 28) {
}

void CEndGameCredits::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	file->writePoint(_frameRange, indent);

	CGameObject::save(file, indent);
}

void CEndGameCredits::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	_frameRange = file->readPoint();

	CGameObject::load(file);
}

bool CEndGameCredits::ActMsg(CActMsg *msg) {
	if (!_flag) {
		if (msg->_action == "ExplodeCredits")
			_frameRange = Point(0, 27);
		if (msg->_action == "Credits")
			_frameRange = Point(28, 46);

		changeView("TheEnd.Node 4.N");
	}

	return true;
}

bool CEndGameCredits::EnterViewMsg(CEnterViewMsg *msg) {
	playMovie(_frameRange.x, _frameRange.y, MOVIE_NOTIFY_OBJECT);
	return true;
}

bool CEndGameCredits::MovieEndMsg(CMovieEndMsg *msg) {
	if (getMovieFrame() == 46) {
		CVisibleMsg visibleMsg;
		visibleMsg.execute("CreditsBackdrop");
	}

	addTimer(4000, 0);
	return true;
}

bool CEndGameCredits::TimerMsg(CTimerMsg *msg) {
	CActMsg actMsg;
	actMsg.execute("EndCreditsText");
	return true;
}

} // End of namespace Titanic
