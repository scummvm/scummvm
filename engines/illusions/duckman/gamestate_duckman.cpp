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

#include "illusions/duckman/gamestate_duckman.h"
#include "illusions/duckman/illusions_duckman.h"
#include "illusions/resources/scriptresource.h"

namespace Illusions {

Duckman_GameState::Duckman_GameState(IllusionsEngine_Duckman *vm)
	: _vm(vm) {
}

uint32 Duckman_GameState::calcWriteBufferSizeInternal() {
	return
		_vm->_scriptResource->_properties.getSize() +
		_vm->_scriptResource->_blockCounters.getSize();
}

bool Duckman_GameState::readStateInternal(Common::ReadStream *in) {
	return
		_vm->_scriptResource->_properties.readFromStream(in) &&
		_vm->_scriptResource->_blockCounters.readFromStream(in);
}

void Duckman_GameState::writeStateInternal(Common::WriteStream *out) {
	_vm->_scriptResource->_properties.writeToStream(out);
	_vm->_scriptResource->_blockCounters.writeToStream(out);
}

} // End of namespace Illusions
