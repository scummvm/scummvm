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

#include "bagel/spacebar/baglib/area_object.h"

namespace Bagel {
namespace SpaceBar {

/* Private methods*/

CBofSize CBagAreaObject::getSize() const {
	return _xSize;
}

/* Public methods*/

CBagAreaObject::CBagAreaObject() {
	_xObjType = AREA_OBJ;
	setVisible(false);
	CBagObject::setOverCursor(4);
}

CBagAreaObject::~CBagAreaObject() {
	CBagAreaObject::detach();
}

ErrorCode CBagAreaObject::attach() {
	return CBagObject::attach();
}

ErrorCode CBagAreaObject::detach() {
	return CBagObject::detach();
}

CBofRect CBagAreaObject::getRect() {
	const CBofPoint p = getPosition();
	const CBofSize s = getSize();
	CBofRect r = CBofRect(p, s);
	return r;
}

void CBagAreaObject::setSize(const CBofSize &xSize) {
	_xSize = xSize;
}

} // namespace SpaceBar
} // namespace Bagel
