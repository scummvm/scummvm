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

#include "titanic/true_talk/tt_input.h"
#include "titanic/true_talk/script_handler.h"

namespace Titanic {

TTinputSubBase::TTinputSubBase() : _field0(0), _field4(0), _field8(0),
	_fieldC(0), _field10(0), _field14(0), _field18(0), _field1C(0),
	_field20(0), _field24(0) {
}

/*------------------------------------------------------------------------*/

TTinput::TTinput(int inputCtr, const TTstring &line, CScriptHandler *owner,
		TTroomScript *roomScript, TTnpcScript *npcScript) :
		_owner(owner), _field2C(1), _inputCtr(inputCtr), _field34(0),
		_field38(0), _initialLine(line), _field4C(0), _roomScript(roomScript),
		_npcScript(npcScript), _field58(0), _field5C(0) {
	_status = _initialLine.isValid() && _normalizedLine.isValid() ? SS_11: SS_VALID;
}

void TTinput::set38(int val) {
	_field38 = val;
}

} // End of namespace Titanic
