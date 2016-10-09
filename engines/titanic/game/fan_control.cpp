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

#include "titanic/game/fan_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CFanControl, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

CFanControl::CFanControl() : CGameObject(), _state(-1),
		_enabled(false), _fieldC4(0), _fieldC8(false), _fieldCC(0) {
}

void CFanControl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_state, indent);
	file->writeNumberLine(_enabled, indent);
	file->writeNumberLine(_fieldC4, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeNumberLine(_fieldCC, indent);

	CGameObject::save(file, indent);
}

void CFanControl::load(SimpleFile *file) {
	file->readNumber();
	_state = file->readNumber();
	_enabled = file->readNumber();
	_fieldC4 = file->readNumber();
	_fieldC8 = file->readNumber();
	_fieldCC = file->readNumber();

	CGameObject::load(file);
}

bool CFanControl::ActMsg(CActMsg *msg) {
	if (msg->_action == "EnableObject")
		_enabled = true;
	else if (msg->_action == "DisableObject")
		_enabled = false;
	else if (msg->_action == "StarlingsDead") {
		_fieldC4 = 0;
		dec54();
		_fieldCC = 0;
	}

	return true;
}

bool CFanControl::StatusChangeMsg(CStatusChangeMsg *msg) {
	if (!_fieldCC) {
		playSound("z#42.wav");
		if (_enabled) {
			switch (msg->_newStatus) {
			case 1:
				_fieldC8 = !_fieldC8;
				if (_fieldC8) {
					playMovie(6, 8, 0);
					_state = 0;
				} else {
					switch (_state) {
					case 0:
						playMovie(4, 6, 0);
						_state = -1;
						break;
					case 1:
						playMovie(0, 6, 0);
						break;
					case 2:
						playMovie(18, 24, 0);
						playMovie(0, 6, 0);
						break;
					default:
						break;
					}

					_state = -1;
				}
				break;

			case 2:
				if (_fieldC8) {
					_state = (_state + 1) % 4;
					switch (_state) {
					case 0:
						playMovie(18, 24, 0);
						playMovie(0, 4, 0);
						break;
					case 1:
						playMovie(8, 12, 0);
						break;
					case 2:
						if (_fieldC4) {
							inc54();
							_fieldCC = 1;
							playMovie(12, 18, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
						} else {
							playMovie(12, 18, 0);
						}
						break;
					default:
						break;
					}
				}
				break;

			default:
				break;
			}

			CStatusChangeMsg statusMsg;
			statusMsg._newStatus = _state;
			statusMsg.execute("RightFan");
		} else {
			petDisplayMessage(1, FAN_HAS_BLOWN_A_FUSE);
		}
	}

	return true;
}

bool CFanControl::EnterViewMsg(CEnterViewMsg *msg) {
	switch (_state) {
	case 0:
		loadFrame(6);
		break;
	case 1:
		loadFrame(4);
		break;
	case 2:
		loadFrame(0);
		break;
	case 3:
		loadFrame(18);
		break;
	default:
		break;
	}

	return true;
}

bool CFanControl::MovieEndMsg(CMovieEndMsg *msg) {
	addTimer(2000);
	return true;
}

bool CFanControl::TimerMsg(CTimerMsg *msg) {
	CStatusChangeMsg statusMsg;
	statusMsg._newStatus = 1;
	statusMsg.execute("StarlingPuret");
	changeView("PromenadeDeck.Node 3.S");
	changeView("PromenadeDeck.Node 3.E");
	return true;
}

} // End of namespace Titanic
