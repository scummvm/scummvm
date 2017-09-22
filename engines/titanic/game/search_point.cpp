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

#include "titanic/game/search_point.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSearchPoint, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

void CSearchPoint::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_value, indent);
	CGameObject::save(file, indent);
}

void CSearchPoint::load(SimpleFile *file) {
	file->readNumber();
	_value = file->readNumber();
	CGameObject::load(file);
}

bool CSearchPoint::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_value > 0) {
		CGameObject *child = dynamic_cast<CGameObject *>(getFirstChild());
		if (child) {
			child->petAddToInventory();
			CVisibleMsg visibleMsg(true);
			visibleMsg.execute(child->getName());
			playSound(TRANSLATE("z#47.wav", "z#578.wav"));
		}

		if (--_value == 0)
			_cursorId = CURSOR_ARROW;
	}

	return true;
}

} // End of namespace Titanic
