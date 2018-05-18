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

#include "illusions/bbdou/gamestate_bbdou.h"
#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/resources/scriptresource.h"

namespace Illusions {

BBDOU_GameState::BBDOU_GameState(IllusionsEngine_BBDOU *vm)
	: _vm(vm) {
}

uint32 BBDOU_GameState::calcWriteBufferSizeInternal() {
	return
		4 + // uint32 prevSceneId
		_vm->_scriptResource->_properties.getSize() +
		_vm->_scriptResource->_blockCounters.getSize();
}

bool BBDOU_GameState::readStateInternal(Common::ReadStream *in) {
	_vm->_prevSceneId = in->readUint32LE();
	return
		_vm->_scriptResource->_properties.readFromStream(in) &&
		_vm->_scriptResource->_blockCounters.readFromStream(in);
}

void BBDOU_GameState::writeStateInternal(Common::WriteStream *out) {
	out->writeUint32LE(_vm->_prevSceneId);
	_vm->_scriptResource->_properties.writeToStream(out);
	_vm->_scriptResource->_blockCounters.writeToStream(out);
}

} // End of namespace Illusions
