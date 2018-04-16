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
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CFanControl, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

CFanControl::CFanControl() : CGameObject(), _state(-1), _enabled(false),
	_starlings(false), _fanOn(false), _starlingsDying(false) {
}

void CFanControl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_state, indent);
	file->writeNumberLine(_enabled, indent);
	file->writeNumberLine(_starlings, indent);
	file->writeNumberLine(_fanOn, indent);
	file->writeNumberLine(_starlingsDying, indent);

	CGameObject::save(file, indent);
}

void CFanControl::load(SimpleFile *file) {
	file->readNumber();
	_state = file->readNumber();
	_enabled = file->readNumber();
	_starlings = file->readNumber();
	_fanOn = file->readNumber();
	_starlingsDying = file->readNumber();

	CGameObject::load(file);
}

bool CFanControl::ActMsg(CActMsg *msg) {
	if (msg->_action == "EnableObject")
		_enabled = true;
	else if (msg->_action == "DisableObject")
		_enabled = false;
	else if (msg->_action == "StarlingsDead") {
		_starlings = false;
		decTransitions();
		_starlingsDying = false;
	}

	return true;
}

bool CFanControl::StatusChangeMsg(CStatusChangeMsg *msg) {
	if (!_starlingsDying) {
		playSound(TRANSLATE("z#42.wav", "z#743.wav"));
		if (_enabled) {
			switch (msg->_newStatus) {
			case 1:
				// Fan Power button
				_fanOn = !_fanOn;
				if (_fanOn) {
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
				// Fan Speed button
				if (_fanOn) {
					_state = (_state + 1) % 3;
					switch (_state) {
					case 0:
						playMovie(18, 24, 0);
						playMovie(0, 4, 0);
						break;
					case 1:
						playMovie(8, 12, 0);
						break;
					case 2:
						if (_starlings) {
							// It's puret time
							incTransitions();
							_starlingsDying = true;
							playMovie(12, 18, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
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
	case -1:
		// Fan off
		loadFrame(6);
		break;
	case 0:
		// Low speed
		loadFrame(4);
		break;
	case 1:
		// Medium speed
		loadFrame(0);
		break;
	case 2:
		// High speed
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
