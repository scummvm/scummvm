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

#include "titanic/game/belbot_get_light.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBelbotGetLight, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(MovieFrameMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

void CBelbotGetLight::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_value, indent);
	CGameObject::save(file, indent);
}

void CBelbotGetLight::load(SimpleFile *file) {
	file->readNumber();
	_value = file->readString();
	CGameObject::load(file);
}

bool CBelbotGetLight::ActMsg(CActMsg *msg) {
	if (msg->_action == "BellbotGetLight") {
		_value = getFullViewName();
		lockMouse();
		changeView("1stClassState.Node 11.N", "");
	}

	return true;
}

bool CBelbotGetLight::MovieEndMsg(CMovieEndMsg *msg) {
	sleep(1000);
	changeView(_value, "");
	unlockMouse();
	return true;
}

bool CBelbotGetLight::MovieFrameMsg(CMovieFrameMsg *msg) {
	if (msg->_frameNumber == 37) {
		CActMsg actMsg("BellbotGetLight");
		actMsg.execute("Eye1");
	}

	return true;
}

bool CBelbotGetLight::EnterViewMsg(CEnterViewMsg *msg) {
	playMovie(MOVIE_NOTIFY_OBJECT);
	movieEvent(37);
	return true;
}

} // End of namespace Titanic
