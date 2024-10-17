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

#include "voyeur/voyeur.h"

#include "engines/advancedDetector.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "graphics/surface.h"

#include "voyeur/detection.h"

#define MAX_SAVES 99

namespace Voyeur {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COPY_PROTECTION,
		{
			_s("Enable lockout system"),
			_s("Require a lockout code to start the game."),
			"copy_protection",
			false,
			0,
			0
		},
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

uint32 VoyeurEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language VoyeurEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform VoyeurEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

bool VoyeurEngine::getIsDemo() const {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

} // End of namespace Voyeur

class VoyeurMetaEngine : public AdvancedMetaEngine<Voyeur::VoyeurGameDescription> {
public:
	const char *getName() const override {
		return "voyeur";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Voyeur::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const Voyeur::VoyeurGameDescription *desc) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool VoyeurMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool Voyeur::VoyeurEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error VoyeurMetaEngine::createInstance(OSystem *syst, Engine **engine, const Voyeur::VoyeurGameDescription *desc) const {
	*engine = new Voyeur::VoyeurEngine(syst,desc);
	return Common::kNoError;
}

SaveStateList VoyeurMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	Voyeur::VoyeurSavegameHeader header;

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot <= MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (header.read(in)) {
					saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));
				}
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int VoyeurMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

bool VoyeurMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor VoyeurMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Voyeur::VoyeurSavegameHeader header;
		header.read(f, false);
		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header._saveName);
		desc.setThumbnail(header._thumbnail);
		desc.setSaveDate(header._saveYear, header._saveMonth, header._saveDay);
		desc.setSaveTime(header._saveHour, header._saveMinutes);
		desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);

		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(VOYEUR)
	REGISTER_PLUGIN_DYNAMIC(VOYEUR, PLUGIN_TYPE_ENGINE, VoyeurMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(VOYEUR, PLUGIN_TYPE_ENGINE, VoyeurMetaEngine);
#endif
