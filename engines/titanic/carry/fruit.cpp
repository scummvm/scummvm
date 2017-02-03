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

#include "titanic/carry/fruit.h"
#include "titanic/npcs/character.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CFruit, CCarry)
	ON_MESSAGE(UseWithCharMsg)
	ON_MESSAGE(LemonFallsFromTreeMsg)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(FrameMsg)
END_MESSAGE_MAP()

CFruit::CFruit() : CCarry(), _field12C(0),
		_field130(0), _field134(0), _field138(0) {
}

void CFruit::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_field12C, indent);
	file->writeNumberLine(_field130, indent);
	file->writeNumberLine(_field134, indent);
	file->writeNumberLine(_field138, indent);

	CCarry::save(file, indent);
}

void CFruit::load(SimpleFile *file) {
	file->readNumber();
	_field12C = file->readNumber();
	_field130 = file->readNumber();
	_field134 = file->readNumber();
	_field138 = file->readNumber();

	CCarry::load(file);
}

bool CFruit::UseWithCharMsg(CUseWithCharMsg *msg) {
	if (msg->_character->isEquals("Barbot") && msg->_character->_visible) {
		CActMsg actMsg("Fruit");
		actMsg.execute(msg->_character);
		_canTake = false;
		setVisible(false);
		return true;
	} else {
		return CCarry::UseWithCharMsg(msg);
	}
}

bool CFruit::LemonFallsFromTreeMsg(CLemonFallsFromTreeMsg *msg) {
	setVisible(true);
	dragMove(msg->_pt);
	_field130 = 1;
	return true;
}

bool CFruit::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	petAddToInventory();
	return true;
}

bool CFruit::FrameMsg(CFrameMsg *msg) {
	if (_field130) {
		if (_bounds.top > 240) {
			_field130 = 0;
			_field134 = 1;
		}

		makeDirty();
		_bounds.top += 3;
		makeDirty();
	}

	return true;
}

} // End of namespace Titanic
