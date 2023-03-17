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

#include "groovie/saveload.h"
#include "groovie/groovie.h"

#include "common/system.h"
#include "common/substream.h"
#include "common/translation.h"

#define SUPPORTED_SAVEFILE_VERSION 1
// 0 - Just script variables, compatible with the original
// 1 - Added one byte with version number at the beginning

namespace Groovie {

int SaveLoad::getMaximumSlot() {
	return MAX_SAVES - 1;
}

bool SaveLoad::isSlotValid(int slot) {
	return slot >= 0 && slot <= getMaximumSlot();
}

Common::String SaveLoad::getSlotSaveName(const Common::String &target, int slot) {
	Common::String fileName = Common::String::format("%s.%03d", target.c_str(), slot);
	return fileName;
}

SaveStateList SaveLoad::listValidSaves(const Common::String &target) {
	SaveStateList list;

	// some Groovie 2 games use save 0 with a garbage name for internal tracking, other games use slot 0 for Open House mode
	const Common::U32String reservedName = _("Reserved"); // I18N: Savegame default name in Groovie engine
	bool hasReserved = false;

	// Get the list of savefiles
	Common::String pattern = Common::String::format("%s.0##", target.c_str());
	Common::StringArray savefiles = g_system->getSavefileManager()->listSavefiles(pattern);

	// Sort the list of filenames
	sort(savefiles.begin(), savefiles.end());

	// Fill the information for the existing savegames
	Common::StringArray::iterator it = savefiles.begin();
	while (it != savefiles.end()) {
		const char *ext = strrchr(it->c_str(), '.');
		if (!ext)
			continue;

		int slot = atoi(ext + 1);

		if (!isSlotValid(slot))
			continue;

		SaveStateDescriptor descriptor;
		Common::InSaveFile *file = SaveLoad::openForLoading(target, slot, &descriptor);
		if (file) {
			// It's a valid savefile, save the descriptor
			delete file;
			if (slot == 0) {
				hasReserved = true;
				if (descriptor.getDescription() != "OPEN HOUSE" && descriptor.getDescription() != "Open House")
					descriptor.setDescription(reservedName);
			}
			list.push_back(descriptor);
		}
		it++;
	}

	if (!hasReserved) {
		SaveStateDescriptor desc;
		desc.setDescription(reservedName);
		list.push_back(desc);
	}

	return list;
}

Common::InSaveFile *SaveLoad::openForLoading(const Common::String &target, int slot, SaveStateDescriptor *descriptor) {
	// Validate the slot number
	if (!isSlotValid(slot)) {
		return nullptr;
	}

	// Open the savefile
	Common::String savename = getSlotSaveName(target, slot);
	Common::InSaveFile *savefile = g_system->getSavefileManager()->openForLoading(savename);
	if (!savefile) {
		return nullptr;
	}

	// Read the savefile version
	uint8 version;
	if (savefile->size() == 1024) {
		version = 0;
	} else {
		version = savefile->readByte();
	}

	// Verify we can read this version
	if (version > SUPPORTED_SAVEFILE_VERSION) {
		//TODO: show the error about unsupported savefile version
	}

	// Save the current position as the start for the engine data
	int metaDataSize = savefile->pos();

	// Fill the SaveStateDescriptor if it was provided
	if (descriptor) {
		// Initialize the SaveStateDescriptor
		descriptor->setSaveSlot(slot);

		// TODO: Add extra information
		//setSaveDate(int year, int month, int day)
		//setSaveTime(int hour, int min)
		//setPlayTime(int hours, int minutes)

		// Read the savegame description
		Common::String description;
		unsigned char c = 1;
		for (int i = 0; (c != 0) && (i < 15); i++) {
			c = savefile->readByte();
			switch (c) {
				case 0:
					break;
				case 16: // @
				// fall through intended
				case 254: // . (generated when pressing space)
					c = ' ';
					break;
				case 244: // $
					c = 0;
					break;
				default:
					c += 0x30;
			}
			if (c != 0) {
				description += c;
			}
		}
		descriptor->setDescription(description);
	}

	// Return a substream, skipping the metadata
	Common::SeekableSubReadStream *sub = new Common::SeekableSubReadStream(savefile, metaDataSize, savefile->size(), DisposeAfterUse::YES);

	// Move to the beginning of the substream
	sub->seek(0, SEEK_SET);

	return sub;
}

Common::OutSaveFile *SaveLoad::openForSaving(const Common::String &target, int slot) {
	// Validate the slot number
	if (!isSlotValid(slot)) {
		return nullptr;
	}

	// Open the savefile
	Common::String savename = getSlotSaveName(target, slot);
	Common::OutSaveFile *savefile = g_system->getSavefileManager()->openForSaving(savename);
	if (!savefile) {
		return nullptr;
	}

	// Write the savefile version
	savefile->writeByte(SUPPORTED_SAVEFILE_VERSION);

	return savefile;
}

} // End of Groovie namespace
