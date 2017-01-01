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
	_fieldBC(0), _fieldC0(0), _fieldC4(0), _fieldC8(0),
	_arrayIndex(0), _string1("NULL"), _fieldFC(0), _field118(0) {
}

void CGondolierSlider::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_fieldC0, indent);
	file->writeNumberLine(_fieldC4, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeNumberLine(_sliderRect1.left, indent);
	file->writeNumberLine(_sliderRect1.top, indent);
	file->writeNumberLine(_sliderRect1.right, indent);
	file->writeNumberLine(_sliderRect1.bottom, indent);
	file->writeNumberLine(_sliderRect2.left, indent);
	file->writeNumberLine(_sliderRect2.top, indent);
	file->writeNumberLine(_sliderRect2.right, indent);
	file->writeNumberLine(_sliderRect2.bottom, indent);
	file->writeNumberLine(_sliderRect1.left, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeNumberLine(_fieldFC, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeNumberLine(_field118, indent);

	CGondolierBase::save(file, indent);
}

void CGondolierSlider::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_fieldC0 = file->readNumber();
	_fieldC4 = file->readNumber();
	_fieldC8 = file->readNumber();
	_sliderRect1.left = file->readNumber();
	_sliderRect1.top = file->readNumber();
	_sliderRect1.right = file->readNumber();
	_sliderRect1.bottom = file->readNumber();
	_sliderRect2.left = file->readNumber();
	_sliderRect2.top = file->readNumber();
	_sliderRect2.right = file->readNumber();
	_sliderRect2.bottom = file->readNumber();
	_arrayIndex = file->readNumber();
	_string1 = file->readString();
	_fieldFC = file->readNumber();
	_string2 = file->readString();
	_string3 = file->readString();
	_field118 = file->readNumber();

	CGondolierBase::load(file);
}

bool CGondolierSlider::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_v1)
		return false;
	if (_fieldFC ? _v5 : _v8)
		return false;

	return _sliderRect1.contains(msg->_mousePos);
}

bool CGondolierSlider::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	if (!(_fieldFC ? _v5 : _v8)) {
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
	if (_fieldFC ? _v5 : _v8)
		return false;

	_field118 = checkStartDragging(msg);
	return _field118;
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
	_field118 = false;
	return true;
}

bool CGondolierSlider::IsHookedOnMsg(CIsHookedOnMsg *msg) {
	if (_fieldFC ? _v5 : _v8)
		return false;

	if (!_sliderRect1.intersects(msg->_rect)) {
		_string2 = CString();
		msg->_result = false;
	} else {
		_string2 = _string1;
		if (_fieldFC) {
			_v5 = _v9 = 1;
		} else {
			_v8 = _v10 = 1;
		}

		msg->_result = true;
	}

	return true;
}

bool CGondolierSlider::FrameMsg(CFrameMsg *msg) {
	if (_fieldFC ? _v5 : _v8) {
		if (_arrayIndex < 10) {
			++_arrayIndex;
			CSignalObject signalMsg;
			signalMsg.execute(this);

			int yp = 0;
			if (_arrayIndex > 0)
				yp = ARRAY[_arrayIndex] - ARRAY[_arrayIndex - 1];

			if (!_string2.empty()) {
				CTranslateObjectMsg transMsg;
				transMsg._delta = Point(0, yp);
				transMsg.execute(_string2);
			}
		}
	} else if (_fieldFC ? _v10 : _v9) {
		if (!_field118 && !_puzzleSolved && _arrayIndex > 0) {
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
	signalMsg._strValue = _fieldFC ? "Fly" : "Tos";
	signalMsg.execute(_string3);

	return true;
}

bool CGondolierSlider::ActMsg(CActMsg *msg) {
	if (msg->_action == "Unhook") {
		if (_fieldFC) {
			_v5 = _v9 = 0;
			_v10 = _v8;
		} else {
			_v8 = _v10 = 0;
			_v9 = _v5;
		}
	}

	return true;
}

} // End of namespace Titanic
