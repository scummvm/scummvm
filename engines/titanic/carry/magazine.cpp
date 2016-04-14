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

#include "titanic/carry/magazine.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMagazine, CCarry)
	ON_MESSAGE(UseWithCharMsg)
	ON_MESSAGE(MouseDoubleClickMsg)
	ON_MESSAGE(VisibleMsg)
	ON_MESSAGE(UseWithOtherMsg)
END_MESSAGE_MAP()

CMagazine::CMagazine() : CCarry() {
}

void CMagazine::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeNumberLine(_field130, indent);

	CCarry::save(file, indent);
}

void CMagazine::load(SimpleFile *file) {
	file->readNumber();
	_field12C = file->readNumber();
	_field130 = file->readNumber();

	CCarry::load(file);
}

bool CMagazine::UseWithCharMsg(CUseWithCharMsg *msg) {
	// todo
	return true;
}

bool CMagazine::MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) {
	// todo
	return true;
}

bool CMagazine::VisibleMsg(CVisibleMsg *msg) {
	// todo
	return true;
}

bool CMagazine::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	// todo
	return true;
}

} // End of namespace Titanic
