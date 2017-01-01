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

#include "titanic/game/end_explode_ship.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEndExplodeShip, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(MovieFrameMsg)
END_MESSAGE_MAP()

void CEndExplodeShip::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_value1, indent);
	file->writeNumberLine(_value2, indent);

	CGameObject::save(file, indent);
}

void CEndExplodeShip::load(SimpleFile *file) {
	file->readNumber();
	_value1 = file->readNumber();
	_value2 = file->readNumber();

	CGameObject::load(file);
}

bool CEndExplodeShip::ActMsg(CActMsg *msg) {
	if (msg->_action == "Arm Bomb") {
		_value1 = 1;
	} else if (msg->_action == "Disarm Bomb") {
		_value1 = 0;
	} else if (msg->_action == "TakeOff") {
		loadSound("a#31.wav");
		loadSound("a#14.wav");
		playGlobalSound("a#13.wav", -1, true, true, 0);
		addTimer(1, 10212, 0);
	}

	return true;
}

bool CEndExplodeShip::TimerMsg(CTimerMsg *msg) {
	if (msg->_actionVal == 1) {
		setVisible(true);
		playMovie(0, 449, 0);
		movieEvent(58);
		playMovie(516, _value1 ? 550 : 551, MOVIE_NOTIFY_OBJECT);
	}

	if (msg->_actionVal == 3) {
		setGlobalSoundVolume(-4, 2, -1);
		CActMsg actMsg(_value1 ? "ExplodeCredits" : "Credits");
		actMsg.execute("EndGameCredits");
	}

	if (msg->_action == "Room") {
		playMovie(550, 583, MOVIE_NOTIFY_OBJECT);
		movieEvent(551);
	}

	return true;
}

bool CEndExplodeShip::MovieEndMsg(CMovieEndMsg *msg) {
	if (getMovieFrame() == 550) {
		playSound("z#399.wav");
		startAnimTimer("Boom", 4200, 0);
	} else {
		addTimer(3, 8000, 0);
	}

	return true;
}

bool CEndExplodeShip::MovieFrameMsg(CMovieFrameMsg *msg) {
	if (getMovieFrame() == 58)
		playSound("a#31.wav", 70);
	else if (getMovieFrame() == 551)
		playSound("a#14.wav");

	return true;
}

} // End of namespace Titanic
