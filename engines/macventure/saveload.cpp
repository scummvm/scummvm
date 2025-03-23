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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "macventure/macventure.h"

#include "common/error.h"
#include "common/savefile.h"
#include "engines/savestate.h"
#include "gui/saveload.h"
#include "graphics/thumbnail.h"

namespace MacVenture {

Common::Error MacVentureEngine::loadGameState(int slot) {
	Common::Error res = Common::kReadingFailed;
	saveAutosaveIfEnabled();

	Common::InSaveFile *saveFile = _saveFileMan->openForLoading(getSaveStateName(slot));

	if (!saveFile)
		return res;

	_world->loadGameFrom(saveFile);
	reset();

	ExtendedSavegameHeader header;
	if (MetaEngine::readSavegameHeader(saveFile, &header))
		setTotalPlayTime(header.playtime);

	res = Common::kNoError;

	delete saveFile;
	return res;
}

Common::Error MacVentureEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::String saveFileName = getSaveStateName(slot);
	Common::OutSaveFile *saveFile = getSaveFileManager()->openForSaving(saveFileName);

	if (!saveFile)
		return Common::kWritingFailed;

	_world->saveGameInto(saveFile);
	getMetaEngine()->appendExtendedSave(saveFile, getTotalPlayTime(), desc, isAutosave);

	saveFile->finalize();
	if (saveFile->err())
		warning("Could not save '%s' correctly.", saveFileName.c_str());

	delete saveFile;
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
		// create our own description for the saved game, the user didn't enter it
		desc = dialog.createDefaultSaveDescription(slot);
	}

	/*
	if (desc.size() > (1 << MACVENTURE_DESC_LENGTH * 8) - 1)
		desc = Common::String(desc.c_str(), (1 << MACVENTURE_DESC_LENGTH * 8) - 1);
	*/
	if (slot < 0)
		return true;

	return saveGameState(slot, desc).getCode() == Common::kNoError;
}

bool MacVentureEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return true;
}

bool MacVentureEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return true;
}

} // End of namespace MacVenture
