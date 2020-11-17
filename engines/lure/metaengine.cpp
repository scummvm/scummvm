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
#include "common/system.h"

#include "engines/advancedDetector.h"

#include "lure/lure.h"
#include "lure/detection.h"

namespace Lure {

uint32 LureEngine::getFeatures() const { return _gameDescription->features; }
Common::Language LureEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform LureEngine::getPlatform() const { return _gameDescription->desc.platform; }

LureLanguage LureEngine::getLureLanguage() const {
	switch (_gameDescription->desc.language) {
	case Common::IT_ITA: return LANG_IT_ITA;
	case Common::FR_FRA: return LANG_FR_FRA;
	case Common::DE_DEU: return LANG_DE_DEU;
	case Common::ES_ESP: return LANG_ES_ESP;
	case Common::RU_RUS: return LANG_RU_RUS;
	case Common::EN_ANY: return LANG_EN_ANY;
	case Common::UNK_LANG: return LANG_UNKNOWN;
	default:
		error("Unknown game language");
	}
}

} // End of namespace Lure

class LureMetaEngine : public AdvancedMetaEngine {
public:
    const char *getName() const override {
		return "lure";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
};

bool LureMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

bool Lure::LureEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error LureMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Lure::LureEngine(syst, (const Lure::LureGameDescription *)desc);
	return Common::kNoError;
}

SaveStateList LureMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = "lure.###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				saveDesc = Lure::getSaveName(in);
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int LureMetaEngine::getMaximumSaveSlot() const { return 999; }

void LureMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = target;
	filename += Common::String::format(".%03d", slot);

	g_system->getSavefileManager()->removeSavefile(filename);
}

#if PLUGIN_ENABLED_DYNAMIC(LURE)
	REGISTER_PLUGIN_DYNAMIC(LURE, PLUGIN_TYPE_ENGINE, LureMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(LURE, PLUGIN_TYPE_ENGINE, LureMetaEngine);
#endif
