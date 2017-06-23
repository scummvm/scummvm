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

#include "titanic/game/wheel_spin_horn.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CWheelSpinHorn, CWheelSpin)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

void CWheelSpinHorn::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_soundName, indent);
	file->writeQuotedLine(_message, indent);

	CWheelSpin::save(file, indent);
}

void CWheelSpinHorn::load(SimpleFile *file) {
	file->readNumber();
	_soundName = file->readString();
	_message = file->readString();

	CWheelSpin::load(file);
}

bool CWheelSpinHorn::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_active) {
		if (!_soundName.empty())
			playSound(_soundName);

		if (!_message.empty())
			petDisplayMessage(_message);

		CActMsg actMsg("Honk");
		actMsg.execute("CaptainsWheel");
	}

	return true;
}

} // End of namespace Titanic
