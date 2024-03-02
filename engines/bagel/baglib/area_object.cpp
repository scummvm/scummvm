/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bagel/baglib/area_object.h"

namespace Bagel {

CBagAreaObject::CBagAreaObject() {
	m_xObjType = AREAOBJ;
	SetVisible(FALSE);
	SetOverCursor(4);
}

CBagAreaObject::~CBagAreaObject() {
	Detach();
}

ERROR_CODE CBagAreaObject::Attach() {
	return CBagObject::Attach();
}

ERROR_CODE CBagAreaObject::Detach() {
	return CBagObject::Detach();
}

CBofRect CBagAreaObject::GetRect(VOID) {
	CBofPoint p = GetPosition();
	CBofSize s = GetSize();
	CBofRect r = CBofRect(p, s);
	return (r);
}

} // namespace Bagel
