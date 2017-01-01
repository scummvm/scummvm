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

#include "titanic/game/wheel_button.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CWheelButton, CBackground)
	ON_MESSAGE(SignalObject)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

CWheelButton::CWheelButton() : CBackground(),
	_fieldE0(false), _timerId(0), _fieldE8(0) {
}

void CWheelButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_timerId, indent);
	file->writeNumberLine(_fieldE8, indent);

	CBackground::save(file, indent);
}

void CWheelButton::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_timerId = file->readNumber();
	_fieldE8 = file->readNumber();

	CBackground::load(file);
}

bool CWheelButton::SignalObject(CSignalObject *msg) {
	bool oldFlag = _fieldE0;
	_fieldE0 = msg->_numValue != 0;

	if (oldFlag != _fieldE0) {
		if (_fieldE0) {
			_timerId = addTimer(500, 500);
		} else {
			stopAnimTimer(_timerId);
			_timerId = 0;
			setVisible(false);
		}
	}

	return true;
}

bool CWheelButton::TimerMsg(CTimerMsg *msg) {
	setVisible(!_visible);
	makeDirty();
	return true;
}

bool CWheelButton::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_timerId) {
		stopAnimTimer(_timerId);
		_timerId = 0;
		setVisible(false);
	}

	return true;
}

} // End of namespace Titanic
