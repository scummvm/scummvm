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

#include "base/plugins.h"
#include "engines/advancedDetector.h"
#include "common/translation.h"

#include "mortevielle/mortevielle.h"
#include "mortevielle/saveload.h"
#include "mortevielle/detection.h"

namespace Mortevielle {

#ifdef USE_TTS
static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_TTS,
		{
			_s("Enable Text to Speech"),
			_s("Use TTS to read text in the game (if TTS is available)"),
			"tts_enabled",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};
#endif


uint32 MortevielleEngine::getGameFlags() const { return _gameDescription->desc.flags; }

Common::Language MortevielleEngine::getLanguage() const { return _gameDescription->desc.language; }

Common::Language MortevielleEngine::getOriginalLanguage() const { return _gameDescription->originalLanguage; }

bool MortevielleEngine::useOriginalData() const { return _gameDescription->dataFeature == kUseOriginalData; }

} // End of namespace Mortevielle

class MortevielleMetaEngine : public AdvancedMetaEngine<Mortevielle::MortevielleGameDescription> {
public:
	const char *getName() const override {
		return "mortevielle";
	}

#ifdef USE_TTS
	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Mortevielle::optionsList;
	}
#endif

	Common::Error createInstance(OSystem *syst, Engine **engine, const Mortevielle::MortevielleGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (!target)
			target = getName();
		if (saveGameIdx == kSavegameFilePattern)
			return Common::String::format("%s.###", target); // There is also sav0.mor for slot 0
		else
			return Mortevielle::MortevielleEngine::generateSaveFilename(target, saveGameIdx);
	}
};

Common::Error MortevielleMetaEngine::createInstance(OSystem *syst, Engine **engine, const Mortevielle::MortevielleGameDescription *desc) const {
	*engine = new Mortevielle::MortevielleEngine(syst,desc);
	return Common::kNoError;
}

bool MortevielleMetaEngine::hasFeature(MetaEngineFeature f) const {
	switch (f) {
	case kSupportsListSaves:
	case kSupportsDeleteSave:
	case kSupportsLoadingDuringStartup:
	case kSavesSupportMetaInfo:
	case kSavesSupportThumbnail:
	case kSavesSupportCreationDate:
	case kSimpleSavesNames:
		return true;
	default:
		return false;
	}
}

int MortevielleMetaEngine::getMaximumSaveSlot() const { return 99; }

SaveStateList MortevielleMetaEngine::listSaves(const char *target) const {
	return Mortevielle::SavegameManager::listSaves(this, target);
}

SaveStateDescriptor MortevielleMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Mortevielle::MortevielleEngine::generateSaveFilename(target, slot);
	return Mortevielle::SavegameManager::querySaveMetaInfos(this, filename);
}

#if PLUGIN_ENABLED_DYNAMIC(MORTEVIELLE)
	REGISTER_PLUGIN_DYNAMIC(MORTEVIELLE, PLUGIN_TYPE_ENGINE, MortevielleMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MORTEVIELLE, PLUGIN_TYPE_ENGINE, MortevielleMetaEngine);
#endif
