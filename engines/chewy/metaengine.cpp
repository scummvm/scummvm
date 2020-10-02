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
#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "chewy/chewy.h"
#include "chewy/detection.h"

namespace Chewy {

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

    bool hasFeature(MetaEngineFeature f) const override;
	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool ChewyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

bool Chewy::ChewyEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool ChewyMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Chewy::ChewyGameDescription *gd = (const Chewy::ChewyGameDescription *)desc;
	if (gd) {
		*engine = new Chewy::ChewyEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList ChewyMetaEngine::listSaves(const char *target) const {
	SaveStateList saveList;

	return saveList;
}

int ChewyMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void ChewyMetaEngine::removeSaveState(const char *target, int slot) const {
}

SaveStateDescriptor ChewyMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(CHEWY)
	REGISTER_PLUGIN_DYNAMIC(CHEWY, PLUGIN_TYPE_ENGINE, ChewyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CHEWY, PLUGIN_TYPE_ENGINE, ChewyMetaEngine);
#endif
