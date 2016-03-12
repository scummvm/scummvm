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

#include "common/textconsole.h"
#include "titanic/true_talk/tt_script_base.h"

namespace Titanic {

TTScriptBase::TTScriptBase(int v1, const char *charClass, int v2,
		const char *charName, int v3, int v4, int v5, int v6, int v7) :
		_charName(charName), _charClass(charClass), 
		_field4(0), _field8(0), _fieldC(0),
		_field20(0), _field24(0), _field28(0), _field2C(0),
		_field30(0), _field34(0), _field38(0), _field3C(0),
		_field40(0), _field44(0), _field48(0), _status(0) {
	if (!areNamesValid()) {
		if (!v7 || !getStatus()) {
			_field8 = v1;
			_field20 = v3;
			_field24 = v4;
			_field28 = v5;
			_field2C = v6;
			_field30 = v7;
			_field34 = v2;
		} else {
			_status = 5;
		}
	}

	if (_status)
		reset();
}

bool TTScriptBase::areNamesValid() {
	bool result = !_charName.isValid() && !_charClass.isValid();
	_status = result ? 0 : 11;
	return result;
}

void TTScriptBase::reset() {
	_field4 = 0;
	_field8 = 4;
	_fieldC = 0;
	_field20 = 0;
	_field24 = -1;
	_field28 = -1;
	_field2C = -1;
	_field30 = 0;
	_field34 = 0;
	_field38 = 0;
	_field3C = 0;
	_field40 = 0;
	_field44 = 0;
	_field48 = 0;
}

void TTScriptBase::proc2(int v) {
	warning("TODO");
}

void TTScriptBase::proc3(int v) {
	warning("TODO");
}

void TTScriptBase::proc4(int v) {
	warning("TODO");
}

void TTScriptBase::proc5() {
	warning("TODO");
}

} // End of namespace Titanic
