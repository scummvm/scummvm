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

#include "titanic/game/fan.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CFan, CGameObject)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CFan::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_state, indent);
	file->writeNumberLine(_value2, indent);

	CGameObject::save(file, indent);
}

void CFan::load(SimpleFile *file) {
	file->readNumber();
	_state = file->readNumber();
	_value2 = file->readNumber();

	CGameObject::load(file);
}

bool CFan::EnterViewMsg(CEnterViewMsg *msg) {
	switch (_state) {
	case 0:
		loadFrame(0);
		break;
	case 1:
		playMovie(24, 34, MOVIE_REPEAT);
		break;
	case 2:
		playMovie(63, 65, MOVIE_REPEAT);
		break;
	default:
		break;
	}

	return true;
}

bool CFan::StatusChangeMsg(CStatusChangeMsg *msg) {
	if (msg->_newStatus >= -1 && msg->_newStatus < 3) {
		int oldState = _state;
		_state = msg->_newStatus;
		switch (_state) {
		case -1:
		case 0:
			if (oldState == 0)
				loadFrame(0);
			else if (oldState == 1)
				playMovie(24, 34, MOVIE_STOP_PREVIOUS | MOVIE_NOTIFY_OBJECT);
			else if (oldState == 2) {
				playMovie(66, 79, MOVIE_STOP_PREVIOUS);
				playMovie(24, 34, MOVIE_NOTIFY_OBJECT);
			}
			break;

		case 1:
			if (oldState == 0)
				playMovie(24, 34, MOVIE_REPEAT | MOVIE_STOP_PREVIOUS);
			if (oldState == 2)
				playMovie(66, 79, MOVIE_NOTIFY_OBJECT | MOVIE_STOP_PREVIOUS);
			break;

		case 2:
			if (oldState == 1)
				playMovie(48, 62, MOVIE_NOTIFY_OBJECT | MOVIE_STOP_PREVIOUS);
			break;

		default:
			break;
		}
	}

	msg->execute("PromDeckFanNoises");
	return true;
}

bool CFan::MovieEndMsg(CMovieEndMsg *msg) {
	switch (_state) {
	case -1:
	case 0:
		loadFrame(0);
		break;

	case 1:
		playMovie(24, 34, MOVIE_REPEAT);
		break;

	case 2:
		playMovie(63, 65, MOVIE_REPEAT);
		break;

	default:
		break;
	}

	return true;
}

} // End of namespace Titanic
