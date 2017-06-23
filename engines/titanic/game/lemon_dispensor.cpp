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

#include "titanic/game/lemon_dispensor.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CLemonDispensor, CBackground)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(ChangeSeasonMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

bool CLemonDispensor::_isSummer;
bool CLemonDispensor::_lemonDropped;
int CLemonDispensor::_hitCounter;
CGameObject *CLemonDispensor::_draggingObject;

CLemonDispensor::CLemonDispensor() : CBackground(),
	_unused1(0), _origPt(Point(9, 15)), _onDispensor(false) {
}

void CLemonDispensor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_isSummer, indent);
	file->writeNumberLine(_lemonDropped, indent);
	file->writeNumberLine(_hitCounter, indent);
	file->writeNumberLine(_unused1, indent);
	file->writeNumberLine(_origPt.x, indent);
	file->writeNumberLine(_origPt.y, indent);
	file->writeNumberLine(_onDispensor, indent);

	CBackground::save(file, indent);
}

void CLemonDispensor::load(SimpleFile *file) {
	file->readNumber();
	_isSummer = file->readNumber();
	_lemonDropped = file->readNumber();
	_hitCounter = file->readNumber();
	_unused1 = file->readNumber();
	_origPt.x = file->readNumber();
	_origPt.y = file->readNumber();
	_onDispensor = file->readNumber();
	_draggingObject = nullptr;

	CBackground::load(file);
}

bool CLemonDispensor::FrameMsg(CFrameMsg *msg) {
	if (_lemonDropped || !_isSummer)
		return true;

	if (!_draggingObject) {
		CGameObject *obj = getDraggingObject();
		if (obj && getView() == findView()) {
			if (obj->isEquals("Perch")) {
				petDisplayMessage(1, TOO_SHORT_TO_REACH_BRANCHES);
				return true;
			}

			if (obj->isEquals("LongStick"))
				_draggingObject = obj;
		}
	}

	if (_draggingObject) {
		Point pt(_origPt.x + _draggingObject->_bounds.left,
			_origPt.y + _draggingObject->_bounds.top);
		bool flag = checkPoint(pt, true);

		if (!_onDispensor) {
			if (flag && ++_hitCounter > 10) {
				CLemonFallsFromTreeMsg lemonMsg(pt);
				lemonMsg.execute("Lemon");
				_lemonDropped = true;
			}
			_onDispensor = true;
		} else if (_onDispensor && !flag) {
			_onDispensor = false;
		}
	}

	return true;
}

bool CLemonDispensor::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	_isSummer = msg->_season == "Summer";
	return true;
}

bool CLemonDispensor::LeaveViewMsg(CLeaveViewMsg *msg) {
	_draggingObject = nullptr;
	_hitCounter = 0;
	_onDispensor = false;
	return true;
}

} // End of namespace Titanic
