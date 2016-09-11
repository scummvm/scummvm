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

#include "titanic/moves/exit_tiania.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CExitTiania, CMovePlayerTo)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

CExitTiania::CExitTiania() : CMovePlayerTo(), _fieldC8(0) {
	_viewNames[0] = _viewNames[1] = _viewNames[2] = "NULL";
}

void CExitTiania::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeQuotedLine(_viewNames[0], indent);
	file->writeQuotedLine(_viewNames[1], indent);
	file->writeQuotedLine(_viewNames[2], indent);

	CMovePlayerTo::save(file, indent);
}

void CExitTiania::load(SimpleFile *file) {
	file->readNumber();
	_fieldC8 = file->readNumber();
	_viewNames[0] = file->readString();
	_viewNames[1] = file->readString();
	_viewNames[2] = file->readString();

	CMovePlayerTo::load(file);
}

bool CExitTiania::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (getPassengerClass() == 4) {
		petDisplayMessage(1, TRANSPORT_OUT_OF_ORDER);
	} else {
		lockMouse();
		for (int idx = 0; idx < 3; ++idx)
			changeView(_viewNames[idx]);
		changeView("Titania.Node 16.N");
		changeView("Dome.Node 4.N");
		changeView("Dome.Node 3.N");
		changeView("Dome.Node 3.S");
		unlockMouse();
	}

	return true;
}

} // End of namespace Titanic
