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

#include "phoenixvr/detection.h"
#include "phoenixvr/metaengine.h"
#include "phoenixvr/phoenixvr.h"

namespace PhoenixVR {

static const ADExtraGuiOptionsMap optionsList[] = {
	{GAMEOPTION_ORIGINAL_SAVELOAD,
	 {_s("Use original save/load screens"),
	  _s("Use the original save/load screens instead of the ScummVM ones"),
	  "original_menus",
	  false,
	  0,
	  0}},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR};

} // End of namespace PhoenixVR

const char *PhoenixVRMetaEngine::getName() const {
	return "phoenixvr";
}

const ADExtraGuiOptionsMap *PhoenixVRMetaEngine::getAdvancedExtraGuiOptions() const {
	return PhoenixVR::optionsList;
}

Common::Error PhoenixVRMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new PhoenixVR::PhoenixVREngine(syst, desc);
	return Common::kNoError;
}

SaveStateList PhoenixVRMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern(getSavegameFilePattern(target));

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (const auto &file : filenames) {
		auto dotPos = file.rfind('.');
		if (dotPos == file.npos)
			continue;
		int slotNum = atoi(file.c_str() + dotPos + 1);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			SaveStateDescriptor desc = querySaveMetaInfos(target, slotNum);
			desc.setSaveSlot(slotNum);
			desc.setDeletableFlag(true);
			saveList.push_back(desc);
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor PhoenixVRMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor desc;
	return desc;
}

bool PhoenixVRMetaEngine::hasFeature(MetaEngineFeature f) const {
	return f == kSimpleSavesNames || f == kSupportsListSaves || f == kSupportsLoadingDuringStartup || f == kSavesSupportThumbnail;
}

#if PLUGIN_ENABLED_DYNAMIC(PHOENIXVR)
REGISTER_PLUGIN_DYNAMIC(PHOENIXVR, PLUGIN_TYPE_ENGINE, PhoenixVRMetaEngine);
#else
REGISTER_PLUGIN_STATIC(PHOENIXVR, PLUGIN_TYPE_ENGINE, PhoenixVRMetaEngine);
#endif
