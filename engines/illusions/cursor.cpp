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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/illusions.h"
#include "illusions/actor.h"
#include "illusions/cursor.h"
#include "illusions/input.h"

namespace Illusions {

Cursor::Cursor(IllusionsEngine *vm)
	: _vm(vm) {
	_status = 1;
	_control = 0;
	_x = 320;
	_y = 240;
	_cursorNum = 1;
	_field_10 = 0;
	_sequenceId = 0;
}

void Cursor::place(Control *control, uint32 sequenceId) {
	_status = 2;
	_control = control;
	_cursorNum = 1;
	_field_10 = 0;
	_sequenceId = sequenceId;
	_visibleCtr = 0;
	_control->_flags |= 8;
	setActorIndex(_cursorNum, 1, 0);
	_vm->_input->setCursorPosition(_control->_actor->_position);
}

void Cursor::setActorIndex(int a2, int a3, int a4) {
	_control->_actor->_actorIndex = 1;// TODO?!? *((_BYTE *)&stru_42C040[30].y + 2 * ((always0 != 0) + 2 * a2) + a3 + 1);
}

void Cursor::setControl(Control *control) {
	_control = control;
}

void Cursor::show() {
	++_visibleCtr;
	if (_visibleCtr > 0) {
		_control->_flags |= 1;
		_control->_actor->_flags |= Illusions::ACTOR_FLAG_1;
		if (_control->_actor->_frameIndex) {
			_control->_actor->_flags |= Illusions::ACTOR_FLAG_2000;
			_control->_actor->_flags |= Illusions::ACTOR_FLAG_4000;
		}
		_vm->_input->discardAllEvents();
	}
}

void Cursor::hide() {
	--_visibleCtr;
	if (_visibleCtr <= 0) {
		_control->_flags &= ~1;
		_control->_actor->_flags &= ~Illusions::ACTOR_FLAG_1;
	}
}

} // End of namespace Illusions
