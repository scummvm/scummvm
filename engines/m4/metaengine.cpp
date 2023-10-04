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
#include "common/savefile.h"
#include "common/system.h"

#include "m4/metaengine.h"
#include "m4/detection.h"
#include "m4/burger/burger.h"
#include "m4/riddle/riddle.h"

namespace M4 {

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

} // End of namespace M4

const char *M4MetaEngine::getName() const {
	return "m4";
}

const ADExtraGuiOptionsMap *M4MetaEngine::getAdvancedExtraGuiOptions() const {
	return M4::optionsList;
}

Common::Error M4MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const M4::M4GameDescription *gd = (const M4::M4GameDescription *)desc;

	switch (gd->gameType) {
	case M4::GType_Burger:
		*engine = new M4::Burger::BurgerEngine(syst, gd);
		break;
	case M4::GType_Riddle:
		*engine = new M4::Riddle::RiddleEngine(syst, gd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}

	return Common::kNoError;
}

bool M4MetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

SaveStateDescriptor M4MetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String saveName = Common::String::format("%s.%03u", target, slot);
	Common::InSaveFile *save = getOriginalSave(saveName);

	if (save) {
		save->skip(4);
		char saveDesc[32];
		save->read(saveDesc, 32);
		saveDesc[31] = '\0';
		delete save;

		SaveStateDescriptor desc(this, slot, saveDesc);
		return desc;
	} else {
		return AdvancedMetaEngine::querySaveMetaInfos(target, slot);
	}
}

Common::InSaveFile *M4MetaEngine::getOriginalSave(const Common::String &saveName) const {
	Common::InSaveFile *save = g_system->getSavefileManager()->openForLoading(saveName);
	char name[16];

	if (save) {
		if (save->seek(-44, SEEK_END) && save->read(name, 7) == 7 &&
			!strncmp(name, "MIRROR", 7)) {
			save->seek(0);
			return save;
		} else if (save->seek(-44, SEEK_END) && save->read(name, 7) == 7 &&
			!strncmp(name, "FAUCET ", 7)) {
			save->seek(0);
			return save;
		}

		delete save;
	}

	return nullptr;
}

#if PLUGIN_ENABLED_DYNAMIC(M4)
REGISTER_PLUGIN_DYNAMIC(M4, PLUGIN_TYPE_ENGINE, M4MetaEngine);
#else
REGISTER_PLUGIN_STATIC(M4, PLUGIN_TYPE_ENGINE, M4MetaEngine);
#endif
