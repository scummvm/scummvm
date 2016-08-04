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

#include "macventure/macventure.h"

#include "common/error.h"
#include "common/savefile.h"
#include "engines/savestate.h"
#include "gui/saveload.h"

namespace MacVenture {

#define MACVENTURE_SAVE_HEADER MKTAG('M', 'V', 'S', 'S') // (M)ac(V)enture (S)cummVM (S)ave (0x4d565353, uint32)
#define MACVENTURE_SAVE_VERSION 1 //1 BYTE
#define MACVENTURE_DESC_LENGTH 1 //1 BYTE for the description length

SaveStateDescriptor loadMetaData(Common::SeekableReadStream *s, int slot) {
	// Metadata is stored at the end of the file
	// |DESCRIPTION						|
	// |HEADER			|VERSION|DESCLEN|
	s->seek(-(5 + MACVENTURE_DESC_LENGTH), SEEK_END);
	uint32 sig = s->readUint32BE();
	byte version = s->readByte();

	SaveStateDescriptor desc(-1, "");	// init to an invalid save slot

	if (sig != MACVENTURE_SAVE_HEADER || version > MACVENTURE_SAVE_VERSION)
		return desc;

	// Save is valid, set its slot number
	desc.setSaveSlot(slot);

	// Load the description
	Common::String name;
	// Depends on MACVENTURE_DESC_LENGTH
	byte descSize = s->readByte();
	s->seek(-(5 + MACVENTURE_DESC_LENGTH + descSize), SEEK_END);
	for (int i = 0; i < descSize; ++i)
		name += s->readByte();
	desc.setDescription(name);

	return desc;
}

void writeMetaData(Common::OutSaveFile *file, Common::String desc) {
	if (desc.size() >= (1 << (MACVENTURE_DESC_LENGTH * 4))) {
		desc.erase((1 << (MACVENTURE_DESC_LENGTH * 4)) - 1);
	}
	file->writeString(desc);
	file->writeUint32BE(MACVENTURE_SAVE_HEADER);
	file->writeByte(MACVENTURE_SAVE_VERSION);
	file->writeByte(desc.size());
}

Common::Error MacVentureEngine::loadGameState(int slot) {
	Common::String saveFileName = Common::String::format("%s.%03d", _targetName.c_str(), slot);
	Common::InSaveFile *file;
	if(!(file = getSaveFileManager()->openForLoading(saveFileName))) {
		error("missing savegame file %s", saveFileName.c_str());
	}
	_world->loadGameFrom(file);
	reset();
	return Common::kNoError;
}

Common::Error MacVentureEngine::saveGameState(int slot, const Common::String &desc) {
	Common::String saveFileName = Common::String::format("%s.%03d", _targetName.c_str(), slot);
	Common::SaveFileManager *manager = getSaveFileManager();
	// HACK Get a real name!
	Common::OutSaveFile *file = manager->openForSaving(saveFileName);
	_world->saveGameInto(file);
	writeMetaData(file, desc);

	file->finalize();
	if (file->err()) {
		warning("Could not save '%s' correctly.", saveFileName.c_str());
	}
	delete file;
	return Common::kNoError;
}

bool MacVentureEngine::scummVMSaveLoadDialog(bool isSave) {
	if (!isSave) {
		// do loading
		GUI::SaveLoadChooser dialog = GUI::SaveLoadChooser(Common::String("Load game:"), Common::String("Load"), false);
		int slot = dialog.runModalWithCurrentTarget();

		if (slot < 0)
			return true;

		return loadGameState(slot).getCode() == Common::kNoError;
	}

	// do saving
	GUI::SaveLoadChooser dialog = GUI::SaveLoadChooser(Common::String("Save game:"), Common::String("Save"), true);
	int slot = dialog.runModalWithCurrentTarget();
	Common::String desc = dialog.getResultString();

	if (desc.empty()) {
		// create our own description for the saved game, the user didnt enter it
		desc = dialog.createDefaultSaveDescription(slot);
	}

	if (desc.size() > (1 << MACVENTURE_DESC_LENGTH * 4) - 1)
		desc = Common::String(desc.c_str(), (1 << MACVENTURE_DESC_LENGTH * 4) - 1);

	if (slot < 0)
		return true;

	return saveGameState(slot, desc).getCode() == Common::kNoError;
}

bool MacVentureEngine::canLoadGameStateCurrently() {
	return true;
}

bool MacVentureEngine::canSaveGameStateCurrently() {
	return true;
}

} // End of namespace MacVenture
