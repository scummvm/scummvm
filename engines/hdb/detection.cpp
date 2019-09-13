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

#include "common/debug.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"
#include "graphics/thumbnail.h"

#include "hdb/hdb.h"

namespace HDB {

enum HDBGameFeatures {
	GF_HANDANGO = (1 << 0)
};

const char *HDBGame::getGameId() const { return _gameDescription->gameId; }
Common::Platform HDBGame::getPlatform() const { return _gameDescription->platform; }

const char *HDBGame::getGameFile() const {
	return _gameDescription->filesDescriptions[0].fileName;
}

uint32 HDBGame::getGameFlags() const {
	return _gameDescription->flags;
}

bool HDBGame::isDemo() const {
	return (getGameFlags() & ADGF_DEMO);
}

bool HDBGame::isPPC() const {
	return (getPlatform() == Common::kPlatformPocketPC);
}

bool HDBGame::isHandango() const {
	return (getGameFlags() & GF_HANDANGO);
}

} // End of namespace HDB

static const PlainGameDescriptor hdbGames[] = {
	{"hdb", "Hyperspace Delivery Boy!"},
	{0, 0}
};

#define GAMEOPTION_CHEATMODE GUIO_GAMEOPTIONS1

namespace HDB {
static const ADGameDescription gameDescriptions[] = {
	{
		"hdb",
		"",
		AD_ENTRY1s("hyperspace.mpc", "ff8e51d0872736bc6afe87cfcb846b70", 50339161),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GAMEOPTION_CHEATMODE
	},
	{
		"hdb",
		"",
		AD_ENTRY1s("hyperspace.mpc", "39d3c9dbc9614f370ad9430307c043f9", 45645305),
		Common::EN_ANY,
		Common::kPlatformLinux,
		ADGF_TESTING,
		GAMEOPTION_CHEATMODE
	},
	// PocketPC Arm
	{
		"hdb",
		"",
		AD_ENTRY1s("hyperspace.msd", "a62468904beb3efe16d4d64f3955a32e", 6825555),
		Common::EN_ANY,
		Common::kPlatformPocketPC,
		ADGF_TESTING,
		GAMEOPTION_CHEATMODE
	},

	// Demos
	{
		"hdb",
		"Demo",
		AD_ENTRY1s("hyperdemo.mpc", "d8743b3b8be56486bcfb1398b2f2aad4", 13816461),
		Common::EN_ANY,
		Common::kPlatformLinux,
		(ADGF_DEMO | ADGF_TESTING),
		GAMEOPTION_CHEATMODE
	},
	{
		"hdb",
		"Demo",
		AD_ENTRY1s("hyperdemo.mpc", "f3bc878e179f00b8666a9846f3d9f9f5", 5236568),
		Common::EN_ANY,
		Common::kPlatformWindows,
		(ADGF_DEMO | ADGF_TESTING),
		GAMEOPTION_CHEATMODE
	},
	// provided by sev
	{
		"hdb",
		"Demo",
		AD_ENTRY1s("hyperdemo.mpc", "7bc533e8f1866931c884f1bc09353744", 13906865),
		Common::EN_ANY,
		Common::kPlatformWindows,
		(ADGF_DEMO | ADGF_TESTING),
		GAMEOPTION_CHEATMODE
	},
	{
		"hdb",
		"Demo",
		AD_ENTRY1s("hyperdemo.msd", "312525298ca9f5ac904883d1ce19dc0f", 3088651),
		Common::EN_ANY,
		Common::kPlatformPocketPC,
		(ADGF_DEMO | ADGF_TESTING),
		GAMEOPTION_CHEATMODE
	},
	{
		"hdb",
		"Handango Demo",
		AD_ENTRY1s("hyperdemo.msd", "2d4457b284a940b7058b36e5706b9951", 3094241),
		Common::EN_ANY,
		Common::kPlatformPocketPC,
		(ADGF_DEMO | ADGF_TESTING | GF_HANDANGO),
		GAMEOPTION_CHEATMODE
	},
	AD_TABLE_END_MARKER
};
} // End of namespace HDB

static const ADExtraGuiOptionsMap optionsList[] = {
		{
				GAMEOPTION_CHEATMODE,
				{
						_s("Enable cheat mode"),
						_s("Debug info and level selection becomes available"),
						"hypercheat",
						false
				}
		},

		AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class HDBMetaEngine : public AdvancedMetaEngine {
public:
	HDBMetaEngine() : AdvancedMetaEngine(HDB::gameDescriptions, sizeof(ADGameDescription), hdbGames, optionsList) {
		_singleId = "hdb";
	}

	virtual const char *getName() const {
		return "Hyperspace Delivery Boy!";
	}

	virtual const char *getOriginalCopyright() const {
		return "Hyperspace Delivery Boy! (c) 2001 Monkeystone Games";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	virtual SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool HDBMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportPlayTime);
}

bool HDB::HDBGame::hasFeature(Engine::EngineFeature f) const {
	return (f == kSupportsRTL) ||
		   (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime);
}

void HDBMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

int HDBMetaEngine::getMaximumSaveSlot() const { return 99; }

SaveStateList HDBMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::ScopedPtr<Common::InSaveFile> in(saveFileMan->openForLoading(*file));
			if (in) {
				SaveStateDescriptor desc;
				char mapName[32];
				Graphics::Surface *thumbnail;

				if (!Graphics::loadThumbnail(*in, thumbnail)) {
					warning("Error loading thumbnail for %s", file->c_str());
				}
				desc.setThumbnail(thumbnail);

				uint32 timeSeconds = in->readUint32LE();;
				in->read(mapName, 32);

				debug(1, "mapName: %s playtime: %d", mapName, timeSeconds);

				desc.setSaveSlot(slotNum);
				desc.setPlayTime(timeSeconds * 1000);

				if (slotNum < 8)
					desc.setDescription(Common::String::format("Auto: %s", mapName));
				else
					desc.setDescription(mapName);

				saveList.push_back(desc);
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor HDBMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::ScopedPtr<Common::InSaveFile> in(g_system->getSavefileManager()->openForLoading(Common::String::format("%s.%03d", target, slot)));

	if (in) {
		SaveStateDescriptor desc;
		char mapName[32];
		Graphics::Surface *thumbnail;

		if (!Graphics::loadThumbnail(*in, thumbnail)) {
			warning("Error loading thumbnail");
		}
		desc.setThumbnail(thumbnail);

		uint32 timeSeconds = in->readUint32LE();
		in->read(mapName, 32);

		desc.setSaveSlot(slot);
		desc.setPlayTime(timeSeconds * 1000);
		desc.setDescription(mapName);

		return desc;
	}

	return SaveStateDescriptor();
}

bool HDBMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new HDB::HDBGame(syst, desc);
	}

	return desc != nullptr;
}

#if PLUGIN_ENABLED_DYNAMIC(HDB)
REGISTER_PLUGIN_DYNAMIC(HDB, PLUGIN_TYPE_ENGINE, HDBMetaEngine);
#else
REGISTER_PLUGIN_STATIC(HDB, PLUGIN_TYPE_ENGINE, HDBMetaEngine);
#endif
