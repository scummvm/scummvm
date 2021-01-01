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

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/util.h"

#include "cine/cine.h"
#include "cine/various.h"

#include "cine/detection.h"

namespace Cine {

#define MAX_SAVEGAMES (ARRAYSIZE(Cine::currentSaveName))
#define SAVEGAME_NAME_LEN (sizeof(Cine::currentSaveName[0]))
#define SAVELIST_SIZE (MAX_SAVEGAMES * SAVEGAME_NAME_LEN)

bool CineEngine::mayHave256Colors() const { return getGameType() == Cine::GType_OS && getPlatform() == Common::kPlatformDOS; }
int CineEngine::getGameType() const { return _gameDescription->gameType; }
uint32 CineEngine::getFeatures() const { return _gameDescription->features; }
Common::Language CineEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform CineEngine::getPlatform() const { return _gameDescription->desc.platform; }

} // End of namespace Cine

class CineMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "cine";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	bool hasFeature(MetaEngineFeature f) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	Common::String getSavegameFile(int saveGameIdx, const char *target = nullptr) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool CineMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSavesUseExtendedFormat);
}

bool Cine::CineEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error CineMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Cine::CineEngine(syst, (const Cine::CINEGameDescription *)desc);
	return Common::kNoError;
}

SaveStateList CineMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateList saveList;

	Common::String pattern;

	Common::StringArray::const_iterator file;

	Common::String filename = target;
	filename += ".dir";
	Common::InSaveFile *in = saveFileMan->openForLoading(filename);
	bool foundAutosave = false;
	if (in) {
		typedef char CommandeType[SAVEGAME_NAME_LEN];
		CommandeType saveNames[MAX_SAVEGAMES];

		// Initialize all savegames' descriptions to empty strings
		// so that if the savegames' descriptions can only be partially read from file
		// then the missing ones are correctly set to empty strings.
		memset(saveNames, 0, sizeof(saveNames));

		in->read(saveNames, SAVELIST_SIZE);
		CommandeType saveDesc;

		pattern = target;
		pattern += ".#*";
		Common::StringArray filenames = saveFileMan->listSavefiles(pattern);

		for (file = filenames.begin(); file != filenames.end(); ++file) {
			// Obtain the extension part of the filename, since it corresponds to the save slot number
			Common::String ext = Common::lastPathComponent(*file, '.');
			int slotNum = (int)ext.asUint64();

			if (ext.equals(Common::String::format("%d", slotNum)) &&
				slotNum >= 0 && slotNum < MAX_SAVEGAMES) {
				// Copy the savegame description making sure it ends with a trailing zero
				strncpy(saveDesc, saveNames[slotNum], SAVEGAME_NAME_LEN);
				saveDesc[sizeof(CommandeType) - 1] = 0;

				SaveStateDescriptor saveStateDesc(slotNum, saveDesc);
				saveStateDesc.setAutosave(slotNum == getAutosaveSlot());
				saveStateDesc.setWriteProtectedFlag(saveStateDesc.isAutosave());

				if (saveStateDesc.getDescription().empty()) {
					if (saveStateDesc.isAutosave()) {
						saveStateDesc.setDescription(_("Unnamed autosave"));
					} else {
						saveStateDesc.setDescription(_("Unnamed savegame"));
					}
				}

				if (saveStateDesc.isAutosave()) {
					foundAutosave = true;
				}

				saveList.push_back(saveStateDesc);
			}
		}
	}

	delete in;

	// No saving on empty autosave slot
	if (!foundAutosave) {
		SaveStateDescriptor desc;
		desc.setDescription(_("Empty autosave"));
		desc.setSaveSlot(getAutosaveSlot());
		desc.setWriteProtectedFlag(true);
		saveList.push_back(desc);
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int CineMetaEngine::getMaximumSaveSlot() const { return MAX_SAVEGAMES - 1; }

Common::String CineMetaEngine::getSavegameFile(int saveGameIdx, const char *target) const {
	return Common::String::format("%s.%d", target == nullptr ? getEngineId() : target, saveGameIdx);
}

SaveStateDescriptor CineMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	if (slot < 0 || slot > getMaximumSaveSlot()) {
		// HACK: Try to make SaveLoadChooserGrid::open() not use save slot
		// numbers over the maximum save slot number for "New save".
		SaveStateDescriptor desc;
		desc.setWriteProtectedFlag(true);
		return desc;
	}

	Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(
		getSavegameFile(slot, target)));

	if (f) {
		// Create the return descriptor
		SaveStateDescriptor desc;

		ExtendedSavegameHeader header;
		if (readSavegameHeader(f.get(), &header, false)) {
			parseSavegameHeader(&header, &desc);
			desc.setThumbnail(header.thumbnail);
		} else {
			// Load savegame descriptions from index file
			typedef char CommandeType[SAVEGAME_NAME_LEN];
			CommandeType saveNames[MAX_SAVEGAMES];
			memset(saveNames, 0, sizeof(saveNames));

			Common::InSaveFile *in;
			in = g_system->getSavefileManager()->openForLoading(Common::String::format("%s.dir", target));

			if (in) {
				in->read(saveNames, SAVELIST_SIZE);
				delete in;
			}

			saveNames[slot][SAVEGAME_NAME_LEN - 1] = 0;
			Common::String saveNameStr((const char *)saveNames[slot]);
			desc.setDescription(saveNameStr);
		}

		if (desc.getDescription().empty()) {
			desc.setDescription(_("Unnamed savegame"));
		}

		desc.setSaveSlot(slot);
		desc.setAutosave(slot == getAutosaveSlot());
		desc.setWriteProtectedFlag(desc.isAutosave());

		return desc;
	}

	// No saving on empty autosave slot
	if (slot == getAutosaveSlot()) {
		SaveStateDescriptor desc;
		desc.setDescription(_("Empty autosave"));
		desc.setSaveSlot(slot);
		desc.setAutosave(true);		
		desc.setWriteProtectedFlag(true);
		return desc;
	}

	return SaveStateDescriptor();
}

void CineMetaEngine::removeSaveState(const char *target, int slot) const {
	if (slot < 0 || slot >= MAX_SAVEGAMES) {
		return;
	}

	// Load savegame descriptions from index file
	typedef char CommandeType[SAVEGAME_NAME_LEN];
	CommandeType saveNames[MAX_SAVEGAMES];

	// Initialize all savegames' descriptions to empty strings
	// so that if the savegames' descriptions can only be partially read from file
	// then the missing ones are correctly set to empty strings.
	memset(saveNames, 0, sizeof(saveNames));

	Common::InSaveFile *in;
	in = g_system->getSavefileManager()->openForLoading(Common::String::format("%s.dir", target));

	if (!in)
		return;

	in->read(saveNames, SAVELIST_SIZE);
	delete in;

	// Set description for selected slot
	char slotName[SAVEGAME_NAME_LEN];
	slotName[0] = 0;
	Common::strlcpy(saveNames[slot], slotName, SAVEGAME_NAME_LEN);

	// Update savegame descriptions
	Common::String indexFile = Common::String::format("%s.dir", target);
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(indexFile);
	if (!out) {
		warning("Unable to open file %s for saving", indexFile.c_str());
		return;
	}

	out->write(saveNames, SAVELIST_SIZE);
	delete out;

	// Delete save file
	Common::String saveFileName = getSavegameFile(slot, target);

	g_system->getSavefileManager()->removeSavefile(saveFileName);
}

#if PLUGIN_ENABLED_DYNAMIC(CINE)
	REGISTER_PLUGIN_DYNAMIC(CINE, PLUGIN_TYPE_ENGINE, CineMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CINE, PLUGIN_TYPE_ENGINE, CineMetaEngine);
#endif

namespace Cine {

Common::Error CineEngine::loadGameState(int slot) {
	bool gameLoaded = makeLoad(getSaveStateName(slot));

	return gameLoaded ? Common::kNoError : Common::kUnknownError;
}

Common::Error CineEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	if (slot < 0 || slot >= MAX_SAVEGAMES) {
		return Common::kCreatingFileFailed;
	}

	// Load savegame descriptions from index file
	loadSaveDirectory();

	// Set description for selected slot making sure it ends with a trailing zero
	strncpy(currentSaveName[slot], desc.c_str(), sizeof(CommandeType));
	currentSaveName[slot][sizeof(CommandeType) - 1] = 0;

	// Update savegame descriptions
	Common::String indexFile = _targetName + ".dir";

	Common::OutSaveFile *fHandle = _saveFileMan->openForSaving(indexFile);
	if (!fHandle) {
		warning("Unable to open file %s for saving", indexFile.c_str());
		return Common::kUnknownError;
	}

	fHandle->write(currentSaveName, SAVELIST_SIZE);
	delete fHandle;

	// Save game
	makeSave(getSaveStateName(slot), getTotalPlayTime() / 1000, desc, isAutosave);

	checkDataDisk(-1);

	return Common::kNoError;
}

Common::String CineEngine::getSaveStateName(int slot) const {
	return getMetaEngine().getSavegameFile(slot, _targetName.c_str());
}

bool CineEngine::canLoadGameStateCurrently() {
	return (!disableSystemMenu && !inMenu);
}

bool CineEngine::canSaveGameStateCurrently() {
	return (allowPlayerInput && !disableSystemMenu && !inMenu);
}

} // End of namespace Cine

