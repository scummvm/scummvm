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

#include "titanic/game/wheel_spin.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CWheelSpin, CBackground)
	ON_MESSAGE(SignalObject)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

void CWheelSpin::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_active, indent);
	CBackground::save(file, indent);
}

void CWheelSpin::load(SimpleFile *file) {
	file->readNumber();
	_active = file->readNumber();
	CBackground::load(file);
}

bool CWheelSpin::SignalObject(CSignalObject *msg) {
	_active = msg->_numValue != 0;
	setVisible(_active);
	return true;
}

bool CWheelSpin::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_active) {
		CActMsg actMsg("Spin");
		actMsg.execute("CaptainsWheel");
	}

	return true;
}

} // End of namespace Titanic
