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
 * Copyright 2020 Google
 *
 */
#include "common/system.h"
#include "common/savefile.h"

#include "hadesch/hadesch.h"
#include "engines/advancedDetector.h"

class HadeschMetaEngine : public AdvancedMetaEngine {
public:
	bool hasFeature(MetaEngineFeature f) const override {
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

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new Hadesch::HadeschEngine(syst, desc);
		return Common::kNoError;
	}

	const char *getName() const override {
		return "hadesch";
	}
};

#if PLUGIN_ENABLED_DYNAMIC(HADESCH)
	REGISTER_PLUGIN_DYNAMIC(HADESCH, PLUGIN_TYPE_ENGINE, HadeschMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(HADESCH, PLUGIN_TYPE_ENGINE, HadeschMetaEngine);
#endif
