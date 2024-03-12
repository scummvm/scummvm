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

#include "common/system.h"
#include "common/savefile.h"

#include "engines/advancedDetector.h"

#include "wage/wage.h"

namespace Wage {

uint32 WageEngine::getFeatures() {
	return _gameDescription->flags;
}

const char *WageEngine::getGameFile() const {
	return _gameDescription->filesDescriptions[0].fileName;
}

} // End of namespace Wage

class WageMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "wage";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	bool hasFeature(MetaEngineFeature f) const override;
	int getMaximumSaveSlot() const override;
};

bool WageMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

bool Wage::WageEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error WageMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Wage::WageEngine(syst, desc);
	return Common::kNoError;
}

int WageMetaEngine::getMaximumSaveSlot() const { return 999; }

#if PLUGIN_ENABLED_DYNAMIC(WAGE)
	REGISTER_PLUGIN_DYNAMIC(WAGE, PLUGIN_TYPE_ENGINE, WageMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(WAGE, PLUGIN_TYPE_ENGINE, WageMetaEngine);
#endif

namespace Wage {

bool WageEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return true;
}

bool WageEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return true;
}

} // End of namespace Wage
