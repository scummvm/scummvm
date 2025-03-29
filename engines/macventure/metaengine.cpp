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
#include "common/system.h"

#include "macventure/macventure.h"

namespace MacVenture {

const char *MacVentureEngine::getGameFileName() const {
	return _gameDescription->filesDescriptions[0].fileName;
}

} // End of namespace MacVenture


namespace MacVenture {

class MacVentureMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "macventure";
	}

protected:
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;
	int getMaximumSaveSlot() const override;
};

bool MacVentureMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSimpleSavesNames) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSavesUseExtendedFormat);
}

bool MacVentureEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

int MacVentureMetaEngine::getMaximumSaveSlot() const { return 999; }

Common::Error MacVentureMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *game) const {
	*engine = new MacVenture::MacVentureEngine(syst, game);
	return Common::kNoError;
}

} // End of namespace MacVenture

#if PLUGIN_ENABLED_DYNAMIC(MACVENTURE)
	REGISTER_PLUGIN_DYNAMIC(MACVENTURE, PLUGIN_TYPE_ENGINE, MacVenture::MacVentureMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MACVENTURE, PLUGIN_TYPE_ENGINE, MacVenture::MacVentureMetaEngine);
#endif
