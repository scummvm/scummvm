/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/ambient.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/state.h"

namespace Myst3 {

Ambient::Ambient(Myst3Engine *vm) :
	_vm(vm) {
}

Ambient::~Ambient() {
}

void Ambient::playCurrentNode(uint32 volume, uint32 fadeOutDelay) {
	if (!fadeOutDelay) fadeOutDelay = 1;

	uint32 node = _vm->_state->getLocationNode();
	uint32 room = _vm->_state->getLocationRoom();
	uint32 age = _vm->_state->getLocationAge();

	// Load sound descriptors
	loadNode(node, room, age);

	// Adjust volume
	scaleVolume(volume);

	// Play sounds
	applySounds(fadeOutDelay);
}

void Ambient::scaleVolume(uint32 volume) {
	for (uint i = 0; i < _sounds.size(); i++)
		_sounds[i].volume = _sounds[i].volume * 100 / volume;
}

void Ambient::loadNode(uint32 node, uint32 room, uint32 age) {
	_sounds.clear();

	if (node == 0)
		node = _vm->_state->getLocationNode();

	_vm->_state->setAmbiantPreviousFadeOutDelay(_vm->_state->getAmbiantFadeOutDelay());

	_scriptAge = age;
	_scriptRoom = room;

	_vm->runAmbientScripts(node);

	if (_sounds.size() == 0)
		_vm->runAmbientScripts(32766);
}

void Ambient::applySounds(uint32 fadeOutDelay) {
	// TODO
}

} /* namespace Myst3 */
