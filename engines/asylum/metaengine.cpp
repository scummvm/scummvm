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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/advancedDetector.h"
#include "base/plugins.h"

#include "asylum/asylum.h"


class AsylumMetaEngine : public AdvancedMetaEngine {
public:
	virtual const char *getName() const override {
		return "asylum";
	}

	virtual const char *getOriginalCopyright() const {
		return "Sanitarium (c) ASC Games";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
	virtual bool hasFeature(MetaEngineFeature f) const override;
	virtual SaveStateList listSaves(const char *target) const override;
	virtual int getMaximumSaveSlot() const override;
	virtual void removeSaveState(const char *target, int slot) const override;
};

bool AsylumMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

bool Asylum::AsylumEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsSubtitleOptions);
}

Common::Error AsylumMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Asylum::AsylumEngine(syst, desc);
	}
	return desc ? Common::kNoError : Common::kUnsupportedGameidError;
}

SaveStateList AsylumMetaEngine::listSaves(const char * /*target*/) const {
	error("[AsylumMetaEngine::listSaves] Not implemented");
}

int AsylumMetaEngine::getMaximumSaveSlot() const {
	error("[AsylumMetaEngine::getMaximumSaveSlot] Not implemented");
}

void AsylumMetaEngine::removeSaveState(const char * /*target*/, int /*slot*/) const {
	error("[AsylumMetaEngine::removeSaveState] Not implemented");
}


#if PLUGIN_ENABLED_DYNAMIC(ASYLUM)
	REGISTER_PLUGIN_DYNAMIC(ASYLUM, PLUGIN_TYPE_ENGINE, AsylumMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ASYLUM, PLUGIN_TYPE_ENGINE, AsylumMetaEngine);
#endif
