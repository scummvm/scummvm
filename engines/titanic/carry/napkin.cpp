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

#include "titanic/carry/napkin.h"
#include "titanic/carry/chicken.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CNapkin, CCarry)
	ON_MESSAGE(UseWithOtherMsg)
END_MESSAGE_MAP()

CNapkin::CNapkin() : CCarry() {
}

void CNapkin::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CCarry::save(file, indent);
}

void CNapkin::load(SimpleFile *file) {
	file->readNumber();
	CCarry::load(file);
}

bool CNapkin::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CChicken *chicken = dynamic_cast<CChicken *>(msg->_other);
	if (chicken) {
		if (chicken->_condiment != "None" || chicken->_greasy) {
			CActMsg actMsg("Clean");
			actMsg.execute("Chicken");
		} else {
			petDisplayMessage(CHICKEN_IS_CLEAN);
		}
	}

	petAddToInventory();
	return CCarry::UseWithOtherMsg(msg);
}

} // End of namespace Titanic
