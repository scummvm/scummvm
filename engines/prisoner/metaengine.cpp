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

#include "common/translation.h"

#include "prisoner/metaengine.h"
#include "prisoner/detection.h"
#include "prisoner/prisoner.h"

namespace Prisoner {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"original_menus",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Prisoner

const char *PrisonerMetaEngine::getName() const {
	return "prisoner";
}

const ADExtraGuiOptionsMap *PrisonerMetaEngine::getAdvancedExtraGuiOptions() const {
	return Prisoner::optionsList;
}

Common::Error PrisonerMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Prisoner::PrisonerEngine(syst, desc);
	return Common::kNoError;
}

bool PrisonerMetaEngine::hasFeature(MetaEngineFeature f) const {
	//return checkExtendedSaves(f) ||
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
//		(f == kSupportsDeleteSave) ||
	   	(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail);
}

bool Prisoner::PrisonerEngine::hasFeature(EngineFeature f) const {
	return
//		(f == kSupportsRTL) || // TODO: Not yet...
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsSubtitleOptions);
}

SaveStateList PrisonerMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Prisoner::PrisonerEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".???";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Prisoner::PrisonerEngine::readSaveHeader(in, false, header) == Prisoner::PrisonerEngine::kRSHENoError) {
					SaveStateDescriptor desc(this, slotNum, header.description);
					desc.setAutosave(slotNum == getAutosaveSlot());
					saveList.push_back(desc);
				}
				delete in;
			}
		}
	}

	return saveList;
}

int PrisonerMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

bool PrisonerMetaEngine::removeSaveState(const char *target, int slot) const {
	// Slot 0 can't be deleted, it's for restarting the game(s)
	if (slot == 0)
		return false;

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String filename = Prisoner::PrisonerEngine::getSavegameFilename(target, slot);

	saveFileMan->removeSavefile(filename.c_str());

	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		// Rename every slot greater than the deleted slot,
		// Also do not rename quicksaves.
		if (slotNum > slot && slotNum < 990) {
			// FIXME: Our savefile renaming done here is inconsitent with what we do in
			// GUI_v2::deleteMenu. While here we rename every slot with a greater equal
			// number of the deleted slot to deleted slot, deleted slot + 1 etc.,
			// we only rename the following slots in GUI_v2::deleteMenu until a slot
			// is missing.
			saveFileMan->renameSavefile(file->c_str(), filename.c_str());

			filename = Prisoner::PrisonerEngine::getSavegameFilename(target, ++slot);
		}
	}
	return true;
}

SaveStateDescriptor PrisonerMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Prisoner::PrisonerEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		Prisoner::PrisonerEngine::SaveHeader header;
		Prisoner::PrisonerEngine::kReadSaveHeaderError error;

		error = Prisoner::PrisonerEngine::readSaveHeader(in, true, header);
		delete in;

		if (error == Prisoner::PrisonerEngine::kRSHENoError) {
			SaveStateDescriptor desc(this, slot, header.description);

			// The constructor write-protects the autosave slot; only the engine writes there
			desc.setDeletableFlag(false);
			desc.setAutosave(slot == getAutosaveSlot());
			desc.setThumbnail(header.thumbnail);

			return desc;
		}
	}

	return SaveStateDescriptor();
}


#if PLUGIN_ENABLED_DYNAMIC(PRISONER)
REGISTER_PLUGIN_DYNAMIC(PRISONER, PLUGIN_TYPE_ENGINE, PrisonerMetaEngine);
#else
REGISTER_PLUGIN_STATIC(PRISONER, PLUGIN_TYPE_ENGINE, PrisonerMetaEngine);
#endif
