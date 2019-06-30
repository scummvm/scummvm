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

#include "macventure/macventure.h"

namespace MacVenture {

#define ADGF_DEFAULT (ADGF_DROPLANGUAGE|ADGF_DROPPLATFORM|ADGF_MACRESFORK)

#define BASEGAME(n, v, f, md5, s) {n, v, AD_ENTRY1s(f, md5, s), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_DEFAULT, GUIO0()}

static const ADGameDescription gameDescriptions[] = {
	BASEGAME("shadowgate", "Zojoi Rerelease", "Shadowgate.bin", "ebbfbcbf93938bd2900cb0c0213b19ad", 68974), // Zojoi Rerelease
	BASEGAME("deja_vu", "Zojoi Rerelease", "Deja Vu.bin", "5e9f5a8e3c8eb29ed02b34ae5937354f", 69034), // Zojoi Rerelease
	BASEGAME("deja_vu2", "Zojoi Rerelease", "Lost in Las Vegas.bin", "8f8e1d8d41f577ee0fbc03847969af0d", 66520), // Zojoi Rerelease
	AD_TABLE_END_MARKER
};

const char *MacVentureEngine::getGameFileName() const {
	return _gameDescription->filesDescriptions[0].fileName;
}
} // End of namespace MacVenture

static const PlainGameDescriptor macventureGames[] = {
	{ "shadowgate", "Shadowgate" },
	{ "deja_vu", "Deja Vu"},
	{ "deja_vu2", "Deja Vu II"},
	{ 0, 0 }
};

namespace MacVenture {

SaveStateDescriptor loadMetaData(Common::SeekableReadStream *s, int slot, bool skipThumbnail = true);

class MacVentureMetaEngine : public AdvancedMetaEngine {
public:
	MacVentureMetaEngine() : AdvancedMetaEngine(MacVenture::gameDescriptions, sizeof(ADGameDescription), macventureGames) {
		_guiOptions = GUIO0();
		_md5Bytes = 5000000; // TODO: Upper limit, adjust it once all games are added
	}

	const char *getName() const {
		return "MacVenture";
	}
	const char *getOriginalCopyright() const {
		return "(C) ICOM Simulations";
	}

protected:
	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	bool hasFeature(MetaEngineFeature f) const;
	SaveStateList listSaves(const char *target) const;
	int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool MacVentureMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

bool MacVentureEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

SaveStateList MacVentureMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		int slotNum = atoi(file->c_str() + file->size() - 3);
		SaveStateDescriptor desc;
		// Do not allow save slot 0 (used for auto-saving) to be deleted or
		// overwritten.
		desc.setDeletableFlag(slotNum != 0);
		desc.setWriteProtectedFlag(slotNum == 0);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				desc = loadMetaData(in, slotNum);
				if (desc.getSaveSlot() != slotNum) {
					// invalid
					delete in;
					continue;
				}
				saveList.push_back(desc);
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int MacVentureMetaEngine::getMaximumSaveSlot() const { return 999; }

bool MacVentureMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *game) const {
	if (game) {
		*engine = new MacVenture::MacVentureEngine(syst, game);
	}
	return game != 0;
}

void MacVentureMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(Common::String::format("%s.%03d", target, slot));
}


SaveStateDescriptor MacVentureMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateDescriptor desc;
	Common::String saveFileName;
	Common::String pattern = target;
	pattern += ".###";
	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum == slot) {
			saveFileName = *file;
		}
	}

	Common::InSaveFile *in = saveFileMan->openForLoading(saveFileName);
	if (in) {
		desc = loadMetaData(in, slot, false);
		delete in;
		return desc;
	}
	return SaveStateDescriptor(-1, "");
}

} // End of namespace MacVenture

#if PLUGIN_ENABLED_DYNAMIC(MACVENTURE)
 REGISTER_PLUGIN_DYNAMIC(MACVENTURE, PLUGIN_TYPE_ENGINE, MacVenture::MacVentureMetaEngine);
#else
 REGISTER_PLUGIN_STATIC(MACVENTURE, PLUGIN_TYPE_ENGINE, MacVenture::MacVentureMetaEngine);
#endif
