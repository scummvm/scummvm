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

#include "backends/keymapper/hardware-input.h"

#ifdef ENABLE_KEYMAPPER

#include "backends/keymapper/keymapper.h"

namespace Common {

HardwareInputSet::HardwareInputSet(const KeyTableEntry *keys, const ModifierTableEntry *modifiers) {
	addHardwareInputs(keys, modifiers);
}

HardwareInputSet::~HardwareInputSet() {
	List<const HardwareInput *>::const_iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); it++)
		delete *it;
}

void HardwareInputSet::addHardwareInput(const HardwareInput *input) {
	checkForInput(input);
	_inputs.push_back(input);
}

const HardwareInput *HardwareInputSet::findHardwareInput(String id) const {
	List<const HardwareInput *>::const_iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); it++) {
		if ((*it)->id == id)
			return (*it);
	}
	return 0;
}

const HardwareInput *HardwareInputSet::findHardwareInput(const KeyState& keystate) const {
	List<const HardwareInput *>::const_iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); it++) {
		if ((*it)->key == keystate)
			return (*it);
	}
	return 0;
}

void HardwareInputSet::addHardwareInputs(const KeyTableEntry keys[], const ModifierTableEntry modifiers[]) {
	const KeyTableEntry *key;
	const ModifierTableEntry *mod;
	char fullKeyId[50];
	char fullKeyDesc[100];
	uint16 ascii;

	for (mod = modifiers; mod->id; mod++) {
		for (key = keys; key->hwId; key++) {
			ascii = key->ascii;

			if (mod->shiftable && key->shiftable) {
				snprintf(fullKeyId, 50, "%s%c", mod->id, toupper(key->hwId[0]));
				snprintf(fullKeyDesc, 100, "%s%c", mod->desc, toupper(key->desc[0]));
				ascii = toupper(key->ascii);
			} else if (mod->shiftable) {
				snprintf(fullKeyId, 50, "S+%s%s", mod->id, key->hwId);
				snprintf(fullKeyDesc, 100, "Shift+%s%s", mod->desc, key->desc);
			} else {
				snprintf(fullKeyId, 50, "%s%s", mod->id, key->hwId);
				snprintf(fullKeyDesc, 100, "%s%s", mod->desc, key->desc);
			}

			addHardwareInput(new HardwareInput(fullKeyId, KeyState(key->keycode, ascii, mod->flag), fullKeyDesc));
		}
	}
}

void HardwareInputSet::checkForInput(const HardwareInput *input) {
	List<const HardwareInput *>::iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); it++) {
		if ((*it)->id == input->id)
			error("Error adding HardwareInput '%s' - id of %s already in use!", input->description.c_str(), input->id.c_str());
		else if ((*it)->key == input->key)
			error("Error adding HardwareInput '%s' - key already in use!", input->description.c_str());
	}
}

} //namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

