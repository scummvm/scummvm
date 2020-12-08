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

#include "common/savefile.h"

#include "engines/advancedDetector.h"

#include "tinsel/bmv.h"
#include "tinsel/cursor.h"
#include "tinsel/tinsel.h"
#include "tinsel/savescn.h"	// needed by TinselMetaEngine::

#include "tinsel/detection.h"

namespace Tinsel {

uint32 TinselEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 TinselEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Language TinselEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform TinselEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 TinselEngine::getVersion() const {
	return _gameDescription->version;
}

bool TinselEngine::getIsADGFDemo() const {
	return (bool)(_gameDescription->desc.flags & ADGF_DEMO);
}

bool TinselEngine::isV1CD() const {
	return (bool)(_gameDescription->desc.flags & ADGF_CD);
}

} // End of namespace Tinsel

class TinselMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const  override{
		return "tinsel";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

    bool hasFeature(MetaEngineFeature f) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	void removeSaveState(const char *target, int slot) const override;
};

bool TinselMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSimpleSavesNames) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSavesSupportCreationDate);
}

bool Tinsel::TinselEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime);
}

SaveStateDescriptor TinselMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName;
	fileName = Common::String::format("%s.%03u", target, slot);

	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (!file) {
		return SaveStateDescriptor();
	}

	file->readUint32LE();		// skip id
	file->readUint32LE();		// skip size
	uint32 ver = file->readUint32LE();
	char saveDesc[SG_DESC_LEN];
	file->read(saveDesc, sizeof(saveDesc));

	saveDesc[SG_DESC_LEN - 1] = 0;
	SaveStateDescriptor desc(slot, saveDesc);

	int8 tm_year = file->readUint16LE();
	int8 tm_mon = file->readSByte();
	int8 tm_mday = file->readSByte();
	int8 tm_hour = file->readSByte();
	int8 tm_min = file->readSByte();
	file->readSByte(); // skip secs

	desc.setSaveDate(1900 + tm_year, 1 + tm_mon, tm_mday);
	desc.setSaveTime(tm_hour, tm_min);

	if (ver >= 3) {
		uint32 playTime = file->readUint32LE(); // playTime in seconds
		desc.setPlayTime(playTime);
	}

	delete file;
	return desc;
}

namespace Tinsel {
extern int getList(Common::SaveFileManager *saveFileMan, const Common::String &target);
}

SaveStateList TinselMetaEngine::listSaves(const char *target) const {
	Common::String pattern = target;
	pattern = pattern + ".###";
	Common::StringArray files = g_system->getSavefileManager()->listSavefiles(pattern);

	SaveStateList saveList;
	int slotNum = 0;
	for (Common::StringArray::const_iterator file = files.begin(); file != files.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		const Common::String &fname = *file;
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fname);
		if (in) {
			in->readUint32LE();		// skip id
			in->readUint32LE();		// skip size
			in->readUint32LE();		// skip version
			char saveDesc[SG_DESC_LEN];
			in->read(saveDesc, sizeof(saveDesc));

			saveDesc[SG_DESC_LEN - 1] = 0;

			saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
			delete in;
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

Common::Error TinselMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Tinsel::TinselEngine(syst, (const Tinsel::TinselGameDescription *)desc);
	return Common::kNoError;
}

int TinselMetaEngine::getMaximumSaveSlot() const { return 99; }

void TinselMetaEngine::removeSaveState(const char *target, int slot) const {
	Tinsel::setNeedLoad();
	// Same issue here as with loadGameState(): we need the physical savegame
	// slot. Refer to bug #3387551.
	int listSlot = -1;
	const int numStates = Tinsel::getList(g_system->getSavefileManager(), target);
	for (int i = 0; i < numStates; ++i) {
		const char *fileName = Tinsel::ListEntry(i, Tinsel::LE_NAME);
		const int saveSlot = atoi(fileName + strlen(fileName) - 3);

		if (saveSlot == slot) {
			listSlot = i;
			break;
		}
	}

	g_system->getSavefileManager()->removeSavefile(Tinsel::ListEntry(listSlot, Tinsel::LE_NAME));
	Tinsel::setNeedLoad();
	Tinsel::getList(g_system->getSavefileManager(), target);
}

#if PLUGIN_ENABLED_DYNAMIC(TINSEL)
	REGISTER_PLUGIN_DYNAMIC(TINSEL, PLUGIN_TYPE_ENGINE, TinselMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TINSEL, PLUGIN_TYPE_ENGINE, TinselMetaEngine);
#endif

namespace Tinsel {

Common::Error TinselEngine::loadGameState(int slot) {
	// FIXME: Hopefully this is only used when loading games via
	// the launcher, since we do a hacky savegame slot to savelist
	// entry mapping here.
	//
	// You might wonder why is needed and here is the answer:
	// The save/load dialog of the GMM operates with the physical
	// savegame slots, while Tinsel internally uses entry numbers in
	// a savelist (which is sorted latest to first). Now to allow
	// proper loading of (especially Discworld2) saves we need to
	// get a savelist entry number instead of the physical slot.
	//
	// There are different possible solutions:
	//
	// One way to fix this would be to pass the filename instead of
	// the savelist entry number to RestoreGame, though it could make
	// problems how DW2 handles CD switches. Normally DW2 would pass
	// '-2' as slot when it changes CDs.
	//
	// Another way would be to convert all of Tinsel to use physical
	// slot numbers instead of savelist entry numbers for loading.
	// This would also allow '-2' as slot for CD changes without
	// any major hackery.

	int listSlot = -1;
	const int numStates = Tinsel::getList();
	for (int i = 0; i < numStates; ++i) {
		const char *fileName = Tinsel::ListEntry(i, Tinsel::LE_NAME);
		const int saveSlot = atoi(fileName + strlen(fileName) - 3);

		if (saveSlot == slot) {
			listSlot = i;
			break;
		}
	}

	if (listSlot == -1)
		return Common::kUnknownError;	// TODO: proper error code

	RestoreGame(listSlot);
	return Common::kNoError;	// TODO: return success/failure
}

#if 0
Common::Error TinselEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::String saveName = _vm->getSavegameFilename((int16)(slot + 1));
	char saveDesc[SG_DESC_LEN];
	Common::strlcpy(saveDesc, desc, SG_DESC_LEN);
	SaveGame((char *)saveName.c_str(), saveDesc);
	ProcessSRQueue();			// This shouldn't be needed, but for some reason it is...
	return Common::kNoError;	// TODO: return success/failure
}
#endif

bool TinselEngine::canLoadGameStateCurrently() { return !_bmv->MoviePlaying(); }

#if 0
bool TinselEngine::canSaveGameStateCurrently() { return isCursorShown(); }
#endif

} // End of namespace Tinsel
