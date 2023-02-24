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
#include "common/file.h"

#include "graphics/surface.h"

#include "ngi/ngi.h"
#include "ngi/detection.h"
#include "ngi/gameloader.h"

namespace NGI {

uint32 NGIEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

bool NGIEngine::isDemo() {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

Common::Language NGIEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

const char *NGIEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

int NGIEngine::getGameGID() const {
	return _gameDescription->gameId;
}

} // End of namspace Fullpipe

class NGIMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "ngi";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override { return 99; }

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

bool NGIMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) || (f == kSupportsLoadingDuringStartup);
}

bool NGI::NGIEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}


Common::Error NGIMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new NGI::NGIEngine(syst, (const NGI::NGIGameDescription *)desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(NGI)
	REGISTER_PLUGIN_DYNAMIC(NGI, PLUGIN_TYPE_ENGINE, NGIMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(NGI, PLUGIN_TYPE_ENGINE, NGIMetaEngine);
#endif
