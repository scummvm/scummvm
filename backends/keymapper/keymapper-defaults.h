/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KEYMAPPER_DEFAULTS_H
#define KEYMAPPER_DEFAULTS_H

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "common/hash-str.h"

namespace Common {

class KeymapperDefaultBindings : public HashMap<String, StringArray> {
public:
	/**
	 * This sets a default hwInput for a given Keymap Action
	 * @param keymapId String representing Keymap id (Keymap.name)
	 * @param actionId String representing Action id (Action.id)
	 * @param hwInputId String representing the HardwareInput id (HardwareInput.id)
	 */
	void setDefaultBinding(String keymapId, String actionId, String hwInputId) {
		setVal(keymapId + "_" + actionId, hwInputId.empty() ? StringArray() : StringArray(1, hwInputId));
	}

	/**
	* This adds a default hwInput for a given Keymap Action
	* @param keymapId String representing Keymap id (Keymap.name)
	* @param actionId String representing Action id (Action.id)
	* @param hwInputId String representing the HardwareInput id (HardwareInput.id)
	*/
	void addDefaultBinding(String keymapId, String actionId, String hwInputId) {
		// NOTE: addDefaultBinding() cannot be used to remove bindings;
		// use setDefaultBinding() with a nullptr or empty string as hwInputId instead.
		if (hwInputId.empty()) {
			return;
		}

		KeymapperDefaultBindings::iterator it = findDefaultBinding(keymapId, actionId);
		if (it != end()) {
			// Don't allow an input to map to the same action multiple times
			StringArray &itv = it->_value;

			Array<String>::const_iterator found = Common::find(itv.begin(), itv.end(), hwInputId);
			if (found == itv.end()) {
				itv.push_back(hwInputId);
			}
		} else {
			setDefaultBinding(keymapId, actionId, hwInputId);
		}
	}

	/**
	 * This retrieves the assigned default hwKey for a given Keymap Action
	 * @param keymapId String representing Keymap id (Keymap.name)
	 * @param actionId String representing Action id (Action.id)
	 * @return StringArray representing the list of HardwareInput ids (HardwareInput.id) that are mapped to Keymap Action
	 */
	const_iterator findDefaultBinding(String keymapId, String actionId) const {
		return find(keymapId + "_" + actionId);
	}
};

} //namespace Common

#endif // #ifndef KEYMAPPER_DEFAULTS_H
