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

#include "common/savefile.h"
#include "common/translation.h"

#include "mtropolis/mtropolis.h"

#include "gui/saveload.h"

namespace MTropolis {

bool MTropolisEngine::promptSave(ISaveWriter *writer) {
	Common::String desc;
	int slot;

	Common::SharedPtr<GUI::SaveLoadChooser> dialog(new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true));

	slot = dialog->runModalWithCurrentTarget();
	desc = dialog->getResultString();

	if (desc.empty()) {
		// create our own description for the saved game, the user didnt enter it
		desc = dialog->createDefaultSaveDescription(slot);
	}

	if (slot < 0)
		return true;

	Common::String saveFileName = getSaveStateName(slot);
	Common::SharedPtr<Common::OutSaveFile> out(_saveFileMan->openForSaving(saveFileName, false));
	if (!writer->writeSave(out.get()) || out->err())
		warning("An error occurred while writing file '%s'", saveFileName.c_str());

	getMetaEngine()->appendExtendedSave(out.get(), getTotalPlayTime(), desc, false);

	return true;
}

bool MTropolisEngine::promptLoad(ISaveReader *reader) {
	Common::String desc;
	int slot;

	{
		Common::SharedPtr<GUI::SaveLoadChooser> dialog(new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false));
		slot = dialog->runModalWithCurrentTarget();
	}

	if (slot < 0)
		return true;

	Common::String saveFileName = getSaveStateName(slot);
	Common::SharedPtr<Common::InSaveFile> in(_saveFileMan->openForLoading(saveFileName));
	if (!reader->readSave(in.get())) {
		warning("An error occurred while reading file '%s'", saveFileName.c_str());
		return false;
	}

	return true;
}

} // End of namespace MTropolis
