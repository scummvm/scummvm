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

#include "titanic/gfx/toggle_switch.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CToggleSwitch, CGameObject)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(ChildDragStartMsg)
	ON_MESSAGE(ChildDragMoveMsg)
END_MESSAGE_MAP()

CToggleSwitch::CToggleSwitch() : CGameObject(), _pressed(false) {
}

void CToggleSwitch::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_pressed, indent);
	file->writePoint(_pos1, indent);

	CGameObject::save(file, indent);
}

void CToggleSwitch::load(SimpleFile *file) {
	file->readNumber();
	_pressed = file->readNumber();
	_pos1 = file->readPoint();

	CGameObject::load(file);
}

bool CToggleSwitch::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	_pressed = !_pressed;
	if (_pressed)
		setToggleColor(0, 0, 0);
	else
		setToggleColor(0xff, 0xff, 0xff);
	return true;
}

bool CToggleSwitch::ChildDragStartMsg(CChildDragStartMsg *msg) {
	_pos1.x = msg->_mousePos.x - _bounds.left;
	_pos1.y = msg->_mousePos.y - _bounds.top;
	return true;
}

bool CToggleSwitch::ChildDragMoveMsg(CChildDragMoveMsg *msg) {
	setPosition(Point(msg->_mousePos.x - _pos1.x, msg->_mousePos.y - _pos1.y));
	return true;
}

} // End of namespace Titanic
