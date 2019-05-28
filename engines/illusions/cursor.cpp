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

void Cursor::setActorIndex(int actorIndex, int a, int b) {
	static int kCursorMap[13][2][2] = {
		{{ 1,  2}, { 0,  0}},
		{{ 3,  4}, { 0,  0}},
		{{ 5,  6}, {13, 14}},
		{{ 7,  8}, { 0,  0}},
		{{ 9, 10}, { 0,  0}},
		{{11, 12}, { 0,  0}},
		{{ 1,  2}, { 0,  0}},
		{{ 0,  0}, { 0,  0}},
		{{ 0,  0}, { 0,  0}},
		{{15, 16}, { 0,  0}},
		{{17, 18}, { 0,  0}},
		{{19, 20}, { 0,  0}},
		{{21, 22}, { 0,  0}}
	};
	_control->_actor->_actorIndex = kCursorMap[actorIndex - 1][b][a - 1];
}

void Cursor::setControl(Control *control) {
	_control = control;
}

void Cursor::show() {
	++_visibleCtr;
	if (_visibleCtr > 0) {
		_control->_flags |= 1;
		_control->_actor->_flags |= Illusions::ACTOR_FLAG_IS_VISIBLE;
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
		_control->_actor->_flags &= ~Illusions::ACTOR_FLAG_IS_VISIBLE;
	}
}

} // End of namespace Illusions
