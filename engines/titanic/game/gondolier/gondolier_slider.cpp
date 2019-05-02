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

/**
 * Y offsets within slider for each successive thumbnail position
 */
static const int Y_OFFSETS[11] = { 0, 0, 1, 4, 9, 15, 21, 27, 32, 35, 36 };

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
	_sliderIndex(0), _stringUnused("NULL"), _sliderNum(0), _dragging(false) {
}

void CGondolierSlider::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeRect(_rectUnused, indent);
	file->writeRect(_thumbRect, indent);
	file->writeRect(_defaultThumbRect, indent);
	file->writeNumberLine(_sliderIndex, indent);
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
	_thumbRect = file->readRect();
	_defaultThumbRect = file->readRect();
	_sliderIndex = file->readNumber();
	_stringUnused = file->readString();
	_sliderNum = file->readNumber();
	_armName = file->readString();
	_signalTarget = file->readString();
	_dragging = file->readNumber();

	CGondolierBase::load(file);
}

bool CGondolierSlider::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_chestOpen)
		return false;
	if (_sliderNum ? _leftSliderHooked : _rightSliderHooked)
		return false;

	return _thumbRect.contains(msg->_mousePos);
}

bool CGondolierSlider::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	if (!(_sliderNum ? _leftSliderHooked : _rightSliderHooked)) {
		int minVal = 0x7FFFFFFF;
		int foundIndex = -1;
		int yp = (_defaultThumbRect.top + _defaultThumbRect.bottom) / 2
			+ _bounds.top - msg->_mousePos.y;

		for (int idx = 0; idx < 11; ++idx) {
			int yDiff = ABS(yp + Y_OFFSETS[idx]);

			if (yDiff < minVal) {
				minVal = yDiff;
				foundIndex = idx;
			}
		}

		if (foundIndex >= 0) {
			_sliderIndex = foundIndex;
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
	if (!_chestOpen)
		return false;
	if (_sliderNum ? _leftSliderHooked : _rightSliderHooked)
		return false;

	_dragging = checkStartDragging(msg);
	return _dragging;
}

bool CGondolierSlider::StatusChangeMsg(CStatusChangeMsg *msg) {
	_sliderIndex = CLIP(10 - msg->_newStatus, 0, 10);
	_thumbRect = _defaultThumbRect;
	_thumbRect.translate(_bounds.left, _bounds.top);
	_thumbRect.translate(0, Y_OFFSETS[_sliderIndex]);

	loadFrame(_sliderIndex);
	return true;
}

bool CGondolierSlider::MouseDragEndMsg(CMouseDragEndMsg *msg) {
	_dragging = false;
	return true;
}

bool CGondolierSlider::IsHookedOnMsg(CIsHookedOnMsg *msg) {
	if (_sliderNum ? _leftSliderHooked : _rightSliderHooked)
		return false;

	if (!_thumbRect.intersects(msg->_rect)) {
		_armName = CString();
		msg->_isHooked = false;
	} else {
		_armName = msg->_armName;
		if (_sliderNum) {
			_leftSliderHooked = _priorLeftSliderHooked = true;
		} else {
			_rightSliderHooked = _priorRightSliderHooked = true;
		}

		msg->_isHooked = true;
	}

	return true;
}

bool CGondolierSlider::FrameMsg(CFrameMsg *msg) {
	if (_sliderNum ? _leftSliderHooked : _rightSliderHooked) {
		if (_sliderIndex < 10) {
			++_sliderIndex;
			CSignalObject signalMsg;
			signalMsg.execute(this);

			int yp = 0;
			if (_sliderIndex > 0)
				yp = Y_OFFSETS[_sliderIndex] - Y_OFFSETS[_sliderIndex - 1];

			if (!_armName.empty()) {
				CTranslateObjectMsg transMsg;
				transMsg._delta = Point(0, yp);
				transMsg.execute(_armName);
			}
		}
	} else if (_sliderNum ? _priorRightSliderHooked : _priorLeftSliderHooked) {
		if (!_dragging && !_puzzleSolved && _sliderIndex > 0) {
			--_sliderIndex;
			CSignalObject signalMsg;
			signalMsg.execute(this);
		}
	}

	return true;
}

bool CGondolierSlider::SignalObject(CSignalObject *msg) {
	_sliderIndex = CLIP(_sliderIndex, 0, 10);
	_thumbRect = _defaultThumbRect;
	_thumbRect.translate(_bounds.left, _bounds.top);
	_thumbRect.translate(0, Y_OFFSETS[_sliderIndex]);
	loadFrame(_sliderIndex);

	CSignalObject signalMsg;
	signalMsg._numValue = 10 - _sliderIndex;
	signalMsg._strValue = _sliderNum ? "Fly" : "Tos";
	signalMsg.execute(_signalTarget);

	return true;
}

bool CGondolierSlider::ActMsg(CActMsg *msg) {
	if (msg->_action == "Unhook") {
		if (_sliderNum) {
			_leftSliderHooked = _priorLeftSliderHooked = false;
			_priorRightSliderHooked = _rightSliderHooked;
		} else {
			_rightSliderHooked = _priorRightSliderHooked = false;
			_priorLeftSliderHooked = _leftSliderHooked;
		}
	}

	return true;
}

} // End of namespace Titanic
