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

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"

#include "base/plugins.h"

#include "touche/touche.h"

class ToucheMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "touche";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (!target)
			target = getName();
		return Touche::generateGameStateFileName(target, saveGameIdx, saveGameIdx == kSavegameFilePattern);
	}
};

bool ToucheMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

bool Touche::ToucheEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsSubtitleOptions);
}

Common::Error ToucheMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Touche::ToucheEngine(syst, desc->language);
	return Common::kNoError;
}

SaveStateList ToucheMetaEngine::listSaves(const char *target) const {
	Common::String pattern = Touche::generateGameStateFileName(target, 0, true);
	Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
	bool slotsTable[Touche::kMaxSaveStates];
	memset(slotsTable, 0, sizeof(slotsTable));
	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		int slot = Touche::getGameStateFileSlot(file->c_str());
		if (slot >= 0 && slot < Touche::kMaxSaveStates) {
			slotsTable[slot] = true;
		}
	}
	for (int slot = 0; slot < Touche::kMaxSaveStates; ++slot) {
		if (slotsTable[slot]) {
			Common::String file = Touche::generateGameStateFileName(target, slot);
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(file);
			if (in) {
				char description[64];
				Touche::readGameStateDescription(in, description, sizeof(description) - 1);
				if (description[0]) {
					saveList.push_back(SaveStateDescriptor(this, slot, description));
				}
				delete in;
			}
		}
	}
	return saveList;
}

int ToucheMetaEngine::getMaximumSaveSlot() const {
	return Touche::kMaxSaveStates - 1;
}

void ToucheMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Touche::generateGameStateFileName(target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

#if PLUGIN_ENABLED_DYNAMIC(TOUCHE)
	REGISTER_PLUGIN_DYNAMIC(TOUCHE, PLUGIN_TYPE_ENGINE, ToucheMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TOUCHE, PLUGIN_TYPE_ENGINE, ToucheMetaEngine);
#endif
