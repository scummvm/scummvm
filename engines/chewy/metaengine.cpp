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
#include "common/system.h"
#include "common/translation.h"
#include "base/plugins.h"
#include "engines/advancedDetector.h"
#include "chewy/chewy.h"
#include "chewy/detection.h"

namespace Chewy {

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

uint32 ChewyEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language ChewyEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Chewy

class ChewyMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "chewy";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Chewy::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	int getMaximumSaveSlot() const override;
};

bool ChewyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsLoadingDuringStartup) ||
		checkExtendedSaves(f);
}

bool Chewy::ChewyEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error ChewyMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Chewy::ChewyEngine(syst, (const Chewy::ChewyGameDescription *)desc);
	return Common::kNoError;
}

int ChewyMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

#if PLUGIN_ENABLED_DYNAMIC(CHEWY)
	REGISTER_PLUGIN_DYNAMIC(CHEWY, PLUGIN_TYPE_ENGINE, ChewyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CHEWY, PLUGIN_TYPE_ENGINE, ChewyMetaEngine);
#endif
