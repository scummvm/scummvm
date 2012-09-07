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
 *
 */

#include "base/plugins.h"

#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"

#include "hopkins/hopkins.h"

namespace Hopkins {

struct HopkinsGameDescription {
	ADGameDescription desc;
};

uint32 HopkinsEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language HopkinsEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool HopkinsEngine::getIsDemo() const {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

} // End of namespace Hopkins

static const PlainGameDescriptor tonyGames[] = {
	{"hopkins", "Hopkins FBI"},
	{0, 0}
};

#include "hopkins/detection_tables.h"

class HopkinsMetaEngine : public AdvancedMetaEngine {
public:
	HopkinsMetaEngine() : AdvancedMetaEngine(Hopkins::gameDescriptions, sizeof(Hopkins::HopkinsGameDescription), tonyGames) {
	}

	virtual const char *getName() const {
		return "Hopkins Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Hopkins Engine (C) ???";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool HopkinsMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail);
}

bool Hopkins::HopkinsEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool HopkinsMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Hopkins::HopkinsGameDescription *gd = (const Hopkins::HopkinsGameDescription *)desc;
	if (gd) {
		*engine = new Hopkins::HopkinsEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList HopkinsMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = "hopkins.0??";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	// TODO

	return saveList;
}

int HopkinsMetaEngine::getMaximumSaveSlot() const {
	return 99;
}

void HopkinsMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = "todo";

	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor HopkinsMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	// TODO
	return SaveStateDescriptor();
}


#if PLUGIN_ENABLED_DYNAMIC(HOPKINS)
REGISTER_PLUGIN_DYNAMIC(HOPKINS, PLUGIN_TYPE_ENGINE, HopkinsMetaEngine);
#else
REGISTER_PLUGIN_STATIC(HOPKINS, PLUGIN_TYPE_ENGINE, HopkinsMetaEngine);
#endif
