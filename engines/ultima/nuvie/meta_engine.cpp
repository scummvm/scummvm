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

#include "ultima/nuvie/meta_engine.h"
#include "common/translation.h"

namespace Ultima {
namespace Nuvie {

void MetaEngine::listSaves(SaveStateList &saveList) {
	// Check whether there's an entry for the original save slot
	for (SaveStateList::iterator it = saveList.begin(); it != saveList.end(); ++it) {
		if (it->getSaveSlot() == ORIGINAL_SAVE_SLOT)
			return;
	}

	// Add in an entry
	SaveStateDescriptor desc;
	desc.setSaveSlot(ORIGINAL_SAVE_SLOT);
	desc.setDescription(_("Original Save"));
	saveList.push_back(desc);

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
}

} // End of namespace Nuvie
} // End of namespace Ultima
