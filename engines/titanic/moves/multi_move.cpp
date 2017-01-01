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

#include "titanic/moves/multi_move.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMultiMove, CMovePlayerTo)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

CMultiMove::CMultiMove() : CMovePlayerTo() {
}

void CMultiMove::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	for (int idx = 0; idx < 5; ++idx)
		file->writeQuotedLine(_viewNames[idx], indent);

	CMovePlayerTo::save(file, indent);
}

void CMultiMove::load(SimpleFile *file) {
	file->readNumber();
	for (int idx = 0; idx < 5; ++idx)
		_viewNames[idx] = file->readString();

	CMovePlayerTo::load(file);
}

bool CMultiMove::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	lockMouse();

	for (int idx = 0; idx < 5 && _viewNames[idx] != "NULL"; ++idx)
		changeView(_viewNames[idx]);

	unlockMouse();
	return true;
}

} // End of namespace Titanic
