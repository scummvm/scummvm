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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "base/plugins.h"

#include "common/savefile.h"
#include "common/system.h"

#include "engines/advancedDetector.h"

#include "buried/buried.h"

namespace Buried {

bool BuriedEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher)
		|| (f == kSupportsLoadingDuringRuntime)
		|| (f == kSupportsSavingDuringRuntime);
}

bool BuriedEngine::isDemo() const {
	// The trial is a demo for the user's sake, but not internally.
	return (_gameDescription->flags & ADGF_DEMO) != 0 && !isTrial();
}

bool BuriedEngine::isTrial() const {
	return (_gameDescription->flags & GF_TRIAL) != 0;
}

bool BuriedEngine::isTrueColor() const {
	return (_gameDescription->flags & GF_TRUECOLOR) != 0;
}

bool BuriedEngine::isWin95() const {
	return (_gameDescription->flags & GF_WIN95) != 0;
}

bool BuriedEngine::isCompressed() const {
	return (_gameDescription->flags & GF_COMPRESSED) != 0;
}

Common::String BuriedEngine::getEXEName() const {
	return _gameDescription->filesDescriptions[0].fileName;
}

Common::String BuriedEngine::getLibraryName() const {
	return _gameDescription->filesDescriptions[1].fileName;
}

Common::Language BuriedEngine::getLanguage() const {
	return _gameDescription->language;
}

} // End of namespace Buried

class BuriedMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "buried";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	int getMaximumSaveSlot() const override { return 999; }
	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		// We set a standard target because saves are compatible among all versions
		return AdvancedMetaEngine::getSavegameFile(saveGameIdx, "buried");
	}
};

bool BuriedMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		f == kSupportsListSaves ||
		f == kSupportsLoadingDuringStartup ||
		f == kSupportsDeleteSave ||
		f == kSavesSupportMetaInfo ||
		f == kSavesSupportThumbnail ||
		f == kSavesSupportCreationDate ||
		f == kSavesSupportPlayTime ||
		f == kSimpleSavesNames ||
		f == kSavesUseExtendedFormat;
}

Common::Error BuriedMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Buried::BuriedEngine(syst, desc);

	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(BURIED)
	REGISTER_PLUGIN_DYNAMIC(BURIED, PLUGIN_TYPE_ENGINE, BuriedMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(BURIED, PLUGIN_TYPE_ENGINE, BuriedMetaEngine);
#endif
