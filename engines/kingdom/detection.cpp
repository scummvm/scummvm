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
#include "common/file.h"

#include "kingdom/kingdom.h"


namespace Kingdom {
const char *KingdomGame::getGameId() const { return _gameDescription->gameId; }
Common::Platform KingdomGame::getPlatform() const { return _gameDescription->platform; }
}

static const PlainGameDescriptor kingdomGames[] = {
	{"kingdom", "Kingdom: The Far Reaches"},
	{0, 0}
};

namespace Kingdom {

#define MAX_SAVES 99

static const ADGameDescription gameDescriptions[] = {
	// Kingdom PC DOS Demo version, provided by Strangerke
	{
		"kingdom",
		0,
		AD_ENTRY1s("KINGDEMO.EXE", "2ce478fc13086d0ddb02a2867ba307dc", 167154),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},

	// Kingdom PC DOS version v1.26 EN, provided by Strangerke
	{
		"kingdom",
		0,
		AD_ENTRY1s("KINGDOM.EXE", "64d3e03b963396ced402f3dc958765c0", 199693),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Kingdom

class KingdomMetaEngine : public AdvancedMetaEngine {
public:
	KingdomMetaEngine() : AdvancedMetaEngine(Kingdom::gameDescriptions, sizeof(ADGameDescription), kingdomGames) {
		_singleId = "kingdom";
	}

	virtual const char *getName() const {
		return "Kingdom: The Far Reaches' Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Kingdom: The far Reaches (C) 1995 Virtual Image Productions";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual int getMaximumSaveSlot() const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool KingdomMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate);
}

bool KingdomMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc)
		*engine = new Kingdom::KingdomGame(syst, desc);

	return desc != nullptr;
}

int KingdomMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

SaveStateList KingdomMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);

	filenames = saveFileMan->listSavefiles(pattern);

	Kingdom::KingdomSavegameHeader header;

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (Kingdom::KingdomGame::readSavegameHeader(in, header)) {
					saveList.push_back(SaveStateDescriptor(slot, header._saveName));

					header._thumbnail->free();
					delete header._thumbnail;
				}

				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

void KingdomMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor KingdomMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Kingdom::KingdomSavegameHeader header;
		Kingdom::KingdomGame::readSavegameHeader(f, header);
		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(slot, header._saveName);
		desc.setThumbnail(header._thumbnail);
		desc.setSaveDate(header._year, header._month, header._day);
		desc.setSaveTime(header._hour, header._minute);

		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(KINGDOM)
REGISTER_PLUGIN_DYNAMIC(KINGDOM, PLUGIN_TYPE_ENGINE, KingdomMetaEngine);
#else
REGISTER_PLUGIN_STATIC(KINGDOM, PLUGIN_TYPE_ENGINE, KingdomMetaEngine);
#endif
