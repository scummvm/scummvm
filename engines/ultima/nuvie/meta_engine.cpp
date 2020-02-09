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
