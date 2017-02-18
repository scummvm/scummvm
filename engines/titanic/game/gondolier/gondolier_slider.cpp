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

#include "titanic/game/gondolier/gondolier_slider.h"

namespace Titanic {

static const int ARRAY[11] = { 0, 0, 1, 4, 9, 15, 21, 27, 32, 35, 36 };

BEGIN_MESSAGE_MAP(CGondolierSlider, CGondolierBase)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseDragMoveMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MouseDragEndMsg)
	ON_MESSAGE(IsHookedOnMsg)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(SignalObject)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

CGondolierSlider::CGondolierSlider() : CGondolierBase(),
	_arrayIndex(0), _stringUnused("NULL"), _sliderNum(0), _dragging(false) {
}

void CGondolierSlider::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeRect(_rectUnused, indent);
	file->writeRect(_sliderRect1, indent);
	file->writeRect(_sliderRect2, indent);
	file->writeNumberLine(_sliderRect1.left, indent);
	file->writeNumberLine(_sliderRect1.top, indent);
	file->writeNumberLine(_sliderRect1.right, indent);
	file->writeNumberLine(_sliderRect1.bottom, indent);
	file->writeNumberLine(_sliderRect2.left, indent);
	file->writeNumberLine(_sliderRect2.top, indent);
	file->writeNumberLine(_sliderRect2.right, indent);
	file->writeNumberLine(_sliderRect2.bottom, indent);
	file->writeNumberLine(_arrayIndex, indent);
	file->writeQuotedLine(_stringUnused, indent);
	file->writeNumberLine(_sliderNum, indent);
	file->writeQuotedLine(_armName, indent);
	file->writeQuotedLine(_signalTarget, indent);
	file->writeNumberLine(_dragging, indent);

	CGondolierBase::save(file, indent);
}

void CGondolierSlider::load(SimpleFile *file) {
	file->readNumber();
	_rectUnused = file->readRect();
	_sliderRect1 = file->readRect();
	_sliderRect2 = file->readRect();
	_arrayIndex = file->readNumber();
	_stringUnused = file->readString();
	_sliderNum = file->readNumber();
	_armName = file->readString();
	_signalTarget = file->readString();
	_dragging = file->readNumber();

	CGondolierBase::load(file);
}

bool CGondolierSlider::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_v1)
		return false;
	if (_sliderNum ? _rightSliderHooked : _leftSliderHooked)
		return false;

	return _sliderRect1.contains(msg->_mousePos);
}

bool CGondolierSlider::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	if (!(_sliderNum ? _rightSliderHooked : _leftSliderHooked)) {
		int minVal = 0x7FFFFFFF;
		int foundIndex = -1;
		int yp = (_sliderRect2.top + _sliderRect2.bottom) / 2
			+ _bounds.top - msg->_mousePos.y;

		for (int idx = 0; idx < 11; ++idx) {
			int yv = yp + ARRAY[idx];
			if (yv < 0)
				yv = -yv;
			if (yv < minVal) {
				minVal = yv;
				foundIndex = idx;
			}
		}

		if (foundIndex >= 0) {
			_arrayIndex = foundIndex;
			CSignalObject signalMsg;
			signalMsg.execute(this);
		}
	}

	return true;
}

bool CGondolierSlider::EnterViewMsg(CEnterViewMsg *msg) {
	CSignalObject signalMsg;
	signalMsg.execute(this);
	return true;
}

bool CGondolierSlider::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!_v1)
		return false;
	if (_sliderNum ? _rightSliderHooked : _leftSliderHooked)
		return false;

	_dragging = checkStartDragging(msg);
	return _dragging;
}

bool CGondolierSlider::StatusChangeMsg(CStatusChangeMsg *msg) {
	_arrayIndex = CLIP(10 - msg->_newStatus, 0, 10);
	_sliderRect1 = _sliderRect2;
	_sliderRect1.translate(_bounds.left, _bounds.top);
	_sliderRect1.translate(0, ARRAY[_arrayIndex]);

	loadFrame(_arrayIndex);
	return true;
}

bool CGondolierSlider::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	_dragging = false;
	return true;
}

bool CGondolierSlider::IsHookedOnMsg(CIsHookedOnMsg *msg) {
	if (_sliderNum ? _rightSliderHooked : _leftSliderHooked)
		return false;

	if (!_sliderRect1.intersects(msg->_rect)) {
		_armName = CString();
		msg->_isHooked = false;
	} else {
		_armName = msg->_armName;
		if (_sliderNum) {
			_rightSliderHooked = _priorRightSliderHooked = true;
		} else {
			_leftSliderHooked = _priorLeftSliderHooked = true;
		}

		msg->_isHooked = true;
	}

	return true;
}

bool CGondolierSlider::FrameMsg(CFrameMsg *msg) {
	if (_sliderNum ? _rightSliderHooked : _leftSliderHooked) {
		if (_arrayIndex < 10) {
			++_arrayIndex;
			CSignalObject signalMsg;
			signalMsg.execute(this);

			int yp = 0;
			if (_arrayIndex > 0)
				yp = ARRAY[_arrayIndex] - ARRAY[_arrayIndex - 1];

			if (!_armName.empty()) {
				CTranslateObjectMsg transMsg;
				transMsg._delta = Point(0, yp);
				transMsg.execute(_armName);
			}
		}
	} else if (_sliderNum ? _priorLeftSliderHooked : _priorRightSliderHooked) {
		if (!_dragging && !_puzzleSolved && _arrayIndex > 0) {
			CSignalObject signalMsg;
			signalMsg.execute(this);
		}
	}

	return true;
}

bool CGondolierSlider::SignalObject(CSignalObject *msg) {
	_arrayIndex = CLIP(_arrayIndex, 0, 10);
	_sliderRect1 = _sliderRect2;
	_sliderRect1.translate(_bounds.left, _bounds.top);
	_sliderRect1.translate(0, ARRAY[_arrayIndex]);
	loadFrame(_arrayIndex);

	CSignalObject signalMsg;
	signalMsg._numValue = 10 - _arrayIndex;
	signalMsg._strValue = _sliderNum ? "Fly" : "Tos";
	signalMsg.execute(_signalTarget);

	return true;
}

bool CGondolierSlider::ActMsg(CActMsg *msg) {
	if (msg->_action == "Unhook") {
		if (_sliderNum) {
			_rightSliderHooked = _priorRightSliderHooked = false;
			_priorLeftSliderHooked = _leftSliderHooked;
		} else {
			_leftSliderHooked = _priorLeftSliderHooked = false;
			_priorRightSliderHooked = _rightSliderHooked;
		}
	}

	return true;
}

} // End of namespace Titanic
