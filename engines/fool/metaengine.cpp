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

#include "common/punycode.h"
#include "common/savefile.h"
#include "common/translation.h"

#include "fool/detection.h"
#include "fool/fool.h"

namespace Fool {

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

} // End of namespace Fool

class FoolMetaEngine : public AdvancedMetaEngine<Fool::FOOLGameDescription> {
public:
	const char *getName() const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const Fool::FOOLGameDescription *desc) const override;

	/**
	 * Determine whether the engine supports the specified MetaEngine feature.
	 *
	 * Used by e.g. the launcher to determine whether to enable the Load button.
	 */
	bool hasFeature(MetaEngineFeature f) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override { return 999; }
	Common::String getSavegameFile(int saveGameIdx, const char *target) const override;
	bool removeSaveState(const char *target, int slot) const override;

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override;
};

const char *FoolMetaEngine::getName() const {
	return "fool";
}

const ADExtraGuiOptionsMap *FoolMetaEngine::getAdvancedExtraGuiOptions() const {
	return Fool::optionsList;
}

Common::Error FoolMetaEngine::createInstance(OSystem *syst, Engine **engine, const Fool::FOOLGameDescription *desc) const {
	*engine = new Fool::FoolEngine(syst, desc);
	return Common::kNoError;
}

bool FoolMetaEngine::hasFeature(MetaEngineFeature f) const {
	return (f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSupportsLoadingDuringStartup);
}

SaveStateList FoolMetaEngine::listSaves(const char *target) const {
	if (!target)
		target = getName();
	SaveStateList result;

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::StringArray filenames = saveMan->listSavefiles(getSavegameFilePattern(target));
	for (const Common::String &f : filenames) {
		int slot = atoi(f.c_str() + f.size() - 3);
		Common::String descOrig = f.substr(strlen(target) + 1, f.size() - strlen(target) - 1 - 4);
		Common::String desc = Common::punycode_decodefilename(descOrig);
		if (desc.empty())
			desc = descOrig;
		result.push_back(SaveStateDescriptor(this, slot, desc));
	}
	Common::sort(result.begin(), result.end(), SaveStateDescriptorSlotComparator());

	return result;
}

Common::String FoolMetaEngine::getSavegameFile(int saveGameIdx, const char *target) const {
	if (!target)
		target = getName();
	if (saveGameIdx == kSavegameFilePattern)
		return Common::String::format("%s-*.###", target);
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::StringArray arr = saveMan->listSavefiles(Common::String::format("%s-*.%03d", target, saveGameIdx));
	if (!arr.empty()) {
		return arr.front();
	}

	return Common::String::format("%s-game.%03d", target, saveGameIdx);
}

bool FoolMetaEngine::removeSaveState(const char *target, int slot) const {
	return g_system->getSavefileManager()->removeSavefile(getSavegameFile(slot, target));
}


#if PLUGIN_ENABLED_DYNAMIC(FOOL)
REGISTER_PLUGIN_DYNAMIC(FOOL, PLUGIN_TYPE_ENGINE, FoolMetaEngine);
#else
REGISTER_PLUGIN_STATIC(FOOL, PLUGIN_TYPE_ENGINE, FoolMetaEngine);
#endif
