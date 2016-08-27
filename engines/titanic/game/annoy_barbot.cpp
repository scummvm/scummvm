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

#include "titanic/game/annoy_barbot.h"

namespace Titanic {

int CAnnoyBarbot::_v1;

BEGIN_MESSAGE_MAP(CAnnoyBarbot, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

void CAnnoyBarbot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_v1, indent);
	CGameObject::save(file, indent);
}

void CAnnoyBarbot::load(SimpleFile *file) {
	file->readNumber();
	_v1 = file->readNumber();
	CGameObject::load(file);
}

bool CAnnoyBarbot::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if ((++_v1 % 3) == 1) {
		CActMsg actMsg("GoRingBell");
		actMsg.execute("Barbot");
	}

	return true;
}

} // End of namespace Titanic
