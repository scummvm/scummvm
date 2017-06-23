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

#include "titanic/gfx/slider_button.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSliderButton, CSTButton)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseDragMoveMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

CSliderButton::CSliderButton() : CSTButton(), _field114(0),
		_field118(0), _field11C(0) {
}

void CSliderButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field114, indent);
	file->writeNumberLine(_field118, indent);
	file->writeNumberLine(_field11C, indent);
	file->writePoint(_pos1, indent);

	CSTButton::save(file, indent);
}

void CSliderButton::load(SimpleFile *file) {
	file->readNumber();
	_field114 = file->readNumber();
	_field118 = file->readNumber();
	_field11C = file->readNumber();
	_pos1 = file->readPoint();

	CSTButton::load(file);
}

bool CSliderButton::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	_pos1 = msg->_mousePos;
	CStatusChangeMsg changeMsg;
	changeMsg.execute(this);
	return true;
}

bool CSliderButton::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	_pos1 = msg->_mousePos;
	return true;
}

bool CSliderButton::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	_pos1 = msg->_mousePos;
	if (_field118) {
		CStatusChangeMsg changeMsg;
		changeMsg.execute(this);
	}

	return true;
}

bool CSliderButton::StatusChangeMsg(CStatusChangeMsg *msg) {
	CStatusChangeMsg changeMsg;
	changeMsg._oldStatus = _currentStatus;
	_currentStatus = (_pos1.y - _bounds.top) / _field11C;
	changeMsg._newStatus = _currentStatus;
	changeMsg.execute(_actionTarget);
	return true;
}

bool CSliderButton::EnterViewMsg(CEnterViewMsg *msg) {
	return true;
}

} // End of namespace Titanic
