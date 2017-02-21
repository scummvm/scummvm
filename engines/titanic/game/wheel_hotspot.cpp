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

#include "titanic/game/wheel_hotspot.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CWheelHotSpot, CBackground)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(SignalObject)
END_MESSAGE_MAP()

void CWheelHotSpot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_active, indent);
	file->writeNumberLine(_action, indent);

	CBackground::save(file, indent);
}

void CWheelHotSpot::load(SimpleFile *file) {
	file->readNumber();
	_active = file->readNumber();
	_action = (WheelHotspotAction)file->readNumber();

	CBackground::load(file);
}

bool CWheelHotSpot::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_active) {
		CActMsg actMsg;

		switch (_action) {
		case WH_STOP:
			actMsg._action = "Stop";
			actMsg.execute("CaptainsWheel");
			break;

		case WH_CRUISE:
			actMsg._action = "Cruise";
			actMsg.execute("CaptainsWheel");
			break;

		case WH_GO:
			actMsg._action = "Go";
			actMsg.execute("CaptainsWheel");
			break;

		default:
			break;
		}
	} else if (_action == WH_GO) {
		petDisplayMessage(GO_WHERE);
	}

	return true;
}

bool CWheelHotSpot::SignalObject(CSignalObject *msg) {
	_active = msg->_numValue != 0;
	return true;
}

} // End of namespace Titanic
