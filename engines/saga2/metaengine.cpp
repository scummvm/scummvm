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

#include "saga2/saga2.h"
#include "engines/advancedDetector.h"

class Saga2MetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "saga2";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;
};

bool Saga2MetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSimpleSavesNames) ||
		(f == kSavesUseExtendedFormat);
}

Common::Error Saga2MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Saga2::SAGA2GameDescription *gd = (const Saga2::SAGA2GameDescription *)desc;
	*engine = new Saga2::Saga2Engine(syst, gd);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(SAGA2)
REGISTER_PLUGIN_DYNAMIC(SAGA2, PLUGIN_TYPE_ENGINE, Saga2MetaEngine);
#else
REGISTER_PLUGIN_STATIC(SAGA2, PLUGIN_TYPE_ENGINE, Saga2MetaEngine);
#endif
