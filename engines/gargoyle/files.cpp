/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gargoyle/files.h"
#include "gui/saveload.h"
#include "common/translation.h"

namespace Gargoyle {

frefid_t Files::prompt(glui32 usage, FileMode fmode, glui32 rock) {
	switch (usage & fileusage_TypeMask) {
	case fileusage_SavedGame: {
		if (fmode == filemode_Write) {
			// Select a savegame slot
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

			int slot = dialog->runModalWithCurrentTarget();
			if (slot >= 0) {
				Common::String desc = dialog->getResultString();
				return createRef(slot, desc, usage, rock);
			}
		} else if (fmode == filemode_Read) {
			// Load a savegame slot
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);

			int slot = dialog->runModalWithCurrentTarget();
			if (slot >= 0) {
				return createRef(slot, "", usage, rock);
			}
		} else {
			error("Unsupport file mode");
		}
		break;
	}

	case fileusage_Transcript:
		return createRef("transcript.txt", fmode, rock);

	default:
		error("Unsupport file mode");
		break;
	}

	return nullptr;
}

frefid_t Files::createRef(int slot, const Common::String &desc, glui32 usage, glui32 rock) {
	_fileReferences.push_back(FileReference());
	frefid_t fref = &_fileReferences.back();

	fref->_slotNumber = slot;
	fref->_description = desc;
	fref->_textMode = ((usage & fileusage_TextMode) != 0);
	fref->_fileType = (FileUsage)(usage & fileusage_TypeMask);
	return fref;
}

frefid_t Files::createRef(const Common::String &filename, glui32 usage, glui32 rock) {
	_fileReferences.push_back(FileReference());
	frefid_t fref = &_fileReferences.back();

	fref->_filename = filename;
	fref->_textMode = ((usage & fileusage_TextMode) != 0);
	fref->_fileType = (FileUsage)(usage & fileusage_TypeMask);
	return fref;
}

void Files::deleteRef(frefid_t fref) {
	for (uint idx = 0; idx < _fileReferences.size(); ++idx) {
		if (&_fileReferences[idx] == fref) {
			_fileReferences.remove_at(idx);
			return;
		}
	}
}


} // End of namespace Gargoyle
