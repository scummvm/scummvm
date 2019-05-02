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

#include "titanic/game/throw_tv_down_well.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CThrowTVDownWell, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(MovieFrameMsg)
END_MESSAGE_MAP()

void CThrowTVDownWell::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_viewName, indent);
	file->writeNumberLine(_flag, indent);
	CGameObject::save(file, indent);
}

void CThrowTVDownWell::load(SimpleFile *file) {
	file->readNumber();
	_viewName = file->readString();
	_flag = file->readNumber();
	CGameObject::load(file);
}

bool CThrowTVDownWell::ActMsg(CActMsg *msg) {
	if (msg->_action == "ThrowTVDownWell" && !_flag) {
		_viewName = getFullViewName();
		lockMouse();
		addTimer(1, 4000, 0);

		CActMsg actMsg("ThrownTVDownWell");
		actMsg.execute("BOWTelevisionMonitor");
	}

	return true;
}

bool CThrowTVDownWell::EnterViewMsg(CEnterViewMsg *msg) {
	playMovie(MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
	movieEvent(49);
	return true;
}

bool CThrowTVDownWell::MovieEndMsg(CMovieEndMsg *msg) {
	sleep(2000);
	changeView("ParrotLobby.Node 11.N");
	playSound(TRANSLATE("z#471.wav", "z#208.wav"));
	addTimer(2, 7000, 0);
	return true;
}

bool CThrowTVDownWell::TimerMsg(CTimerMsg *msg) {
	if (msg->_actionVal == 1) {
		changeView("ParrotLobby.Node 10.N");
	} else if (msg->_actionVal == 2) {
		playSound(TRANSLATE("z#468.wav", "z#205.wav"), 50);
		sleep(1500);
		changeView(_viewName);
		_viewName = "NULL";
		unlockMouse();
		playSound(TRANSLATE("z#47.wav", "z#578.wav"));
	}

	return true;
}

bool CThrowTVDownWell::MovieFrameMsg(CMovieFrameMsg *msg) {
	playSound(TRANSLATE("z#470.wav", "z#207.wav"));
	return true;
}

} // End of namespace Titanic
