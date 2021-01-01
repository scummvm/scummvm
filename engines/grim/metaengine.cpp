/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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


#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"
#include "engines/grim/emi/emi.h"

#include "common/system.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "common/config-manager.h"

namespace Grim {

class GrimMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "Grim";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine) const override {
		Engines::upgradeTargetIfNecessary(obsoleteGameIDsTable);
		return AdvancedMetaEngine::createInstance(syst, engine);
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	bool hasFeature(MetaEngineFeature f) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;

	SaveStateList listSaves(const char *target) const override;

};

Common::Error GrimMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const GrimGameDescription *gd = (const GrimGameDescription *)desc;

	if (gd->gameType == GType_MONKEY4) {
#ifdef ENABLE_MONKEY4
		*engine = new EMIEngine(syst, gd->desc.flags, gd->gameType, gd->desc.platform, gd->desc.language);
#else
		return Common::Error(Common::kUnsupportedGameidError, _s("Escape from Monkey Island support is not compiled in"));
#endif
	} else {
		*engine = new GrimEngine(syst, gd->desc.flags, gd->gameType, gd->desc.platform, gd->desc.language);
	}

	return Common::kNoError;
}

bool GrimMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup);
}

Common::KeymapArray GrimMetaEngine::initKeymaps(const char *target) const {
	Common::String gameId = ConfMan.get("gameid", target);

#ifdef ENABLE_GRIM
	if (gameId == "grim") {
		return Grim::GrimEngine::initKeymapsGrim(target);
	}
#endif
#ifdef ENABLE_MONKEY4
	if (gameId == "monkey4") {
		return Grim::GrimEngine::initKeymapsEMI(target);
	}
#endif

	return AdvancedMetaEngine::initKeymaps(target);
}

SaveStateList GrimMetaEngine::listSaves(const char *target) const {
	Common::String gameId = ConfMan.get("gameid", target);
	Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", target));
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = gameId == "monkey4" ? "efmi###.gsv" : "grim##.gsv";

	if (platform == Common::kPlatformPS2)
		pattern = "efmi###.ps2";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	char str[256];
	int32 strSize;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + 4);

		if (slotNum >= 0) {
			SaveGame *savedState = SaveGame::openForLoading(*file);
			if (savedState && savedState->isCompatible()) {
				if (platform == Common::kPlatformPS2)
					savedState->beginSection('PS2S');
				else
					savedState->beginSection('SUBS');
				strSize = savedState->readLESint32();
				savedState->read(str, strSize);
				savedState->endSection();
				saveList.push_back(SaveStateDescriptor(slotNum, str));
			}
			delete savedState;
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

} // End of namespace Grim

#if PLUGIN_ENABLED_DYNAMIC(GRIM)
	REGISTER_PLUGIN_DYNAMIC(GRIM, PLUGIN_TYPE_ENGINE, Grim::GrimMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GRIM, PLUGIN_TYPE_ENGINE, Grim::GrimMetaEngine);
#endif
