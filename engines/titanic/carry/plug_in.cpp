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

#include "titanic/carry/plug_in.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPlugIn, CCarry)
	ON_MESSAGE(UseWithOtherMsg)
END_MESSAGE_MAP()

CPlugIn::CPlugIn() : CCarry(), _unused(0) {
}

void CPlugIn::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_unused, indent);
	CCarry::save(file, indent);
}

void CPlugIn::load(SimpleFile *file) {
	file->readNumber();
	_unused = file->readNumber();
	CCarry::load(file);
}

bool CPlugIn::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CGameObject *other = msg->_other;
	CString otherName = other->getName();

	if (otherName == "PET") {
		return CCarry::UseWithOtherMsg(msg);
	} else if (isEquals("DatasideTransporter")) {
		CShowTextMsg textMsg(INCORRECTLY_CALIBRATED);
		textMsg.execute("PET");
	}

	return true;
}

} // End of namespace Titanic
