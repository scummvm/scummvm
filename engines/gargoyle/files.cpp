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
#include "gargoyle/gargoyle.h"
#include "gui/saveload.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/translation.h"

namespace Gargoyle {

frefid_t Files::createByPrompt(glui32 usage, FileMode fmode, glui32 rock) {
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
	frefid_t fref = new FileReference();
	fref->_slotNumber = slot;
	fref->_description = desc;
	fref->_textMode = ((usage & fileusage_TextMode) != 0);
	fref->_fileType = (FileUsage)(usage & fileusage_TypeMask);

	_fileReferences.push_back(FileRefArray::value_type(fref));
	return fref;
}

frefid_t Files::createRef(const Common::String &filename, glui32 usage, glui32 rock) {
	frefid_t fref = new FileReference();
	fref->_filename = filename;
	fref->_textMode = ((usage & fileusage_TextMode) != 0);
	fref->_fileType = (FileUsage)(usage & fileusage_TypeMask);

	_fileReferences.push_back(FileRefArray::value_type(fref));
	return fref;
}

frefid_t Files::createTemp(glui32 usage, glui32 rock) {
	return createRef(Common::String::format("%s.tmp", g_vm->getTargetName().c_str()),
		usage, rock);
}

frefid_t Files::createFromRef(frefid_t fref, glui32 usage, glui32 rock) {
	return createRef(fref->_filename, usage, rock);
}

void Files::deleteRef(frefid_t fref) {
	for (uint idx = 0; idx < _fileReferences.size(); ++idx) {
		if (_fileReferences[idx].get() == fref) {
			_fileReferences.remove_at(idx);
			return;
		}
	}
}

frefid_t Files::iterate(frefid_t fref, glui32 *rock) {
	// Find reference following the specified one
	int index = -1;
	for (uint idx = 0; idx < _fileReferences.size(); ++idx) {
		if (fref == nullptr || _fileReferences[idx].get() == fref) {
			if (idx < (_fileReferences.size() - 1))
				index = idx + 1;
			break;
		}
	}

	if (index != -1) {
		if (rock)
			*rock = _fileReferences[index].get()->_rock;
		return _fileReferences[index].get();
	}

	if (rock)
		*rock = 0;
	return nullptr;
}

/*--------------------------------------------------------------------------*/

const Common::String FileReference::getSaveName() const {
	assert(_slotNumber != -1);
	return Common::String::format("%s.%.3d", g_vm->getTargetName().c_str(), _slotNumber);
}

bool FileReference::exists() const {
	Common::String filename;

	if (_slotNumber == -1) {
		if (Common::File::exists(_filename))
			return true;
		filename = _filename;
	} else {
		filename = getSaveName();
	}

	// Check for a savegame (or other file in the save folder) with that name
	Common::InSaveFile *inSave = g_system->getSavefileManager()->openForLoading(filename);
	bool result = inSave != nullptr;
	delete inSave;
	return result;
}

void FileReference::deleteFile() {
	Common::String filename = (_slotNumber == -1) ? _filename : getSaveName();
	g_system->getSavefileManager()->removeSavefile(filename);
}

} // End of namespace Gargoyle
