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

#include "groovie/groovie.h"
#include "groovie/saveload.h"

#include "common/system.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"
#include "groovie/detection.h"

namespace Groovie {

class GroovieMetaEngine : public AdvancedMetaEngine {
public:
    const char *getName() const override {
		return "groovie";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

Common::Error GroovieMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
#ifndef ENABLE_GROOVIE2
	if (((const GroovieGameDescription *)gd)->version == kGroovieV2)
		return Common::Error(Common::kUnsupportedGameidError, _s("GroovieV2 support is not compiled in"));
#endif

	*engine = new GroovieEngine(syst, (const GroovieGameDescription *)gd);
	return Common::kNoError;
}

bool GroovieMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo);
}

SaveStateList GroovieMetaEngine::listSaves(const char *target) const {
	return SaveLoad::listValidSaves(target);
}

int GroovieMetaEngine::getMaximumSaveSlot() const {
	return SaveLoad::getMaximumSlot();
}

void GroovieMetaEngine::removeSaveState(const char *target, int slot) const {
	if (!SaveLoad::isSlotValid(slot)) {
		// Invalid slot, do nothing
		return;
	}

	Common::String filename = SaveLoad::getSlotSaveName(target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor GroovieMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor desc;

	Common::InSaveFile *savefile = SaveLoad::openForLoading(target, slot, &desc);
	delete savefile;

	return desc;
}

} // End of namespace Groovie

#if PLUGIN_ENABLED_DYNAMIC(GROOVIE)
	REGISTER_PLUGIN_DYNAMIC(GROOVIE, PLUGIN_TYPE_ENGINE, Groovie::GroovieMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GROOVIE, PLUGIN_TYPE_ENGINE, Groovie::GroovieMetaEngine);
#endif
