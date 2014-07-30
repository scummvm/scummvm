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

#include "access/access.h"

#include "base/plugins.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"

#define MAX_SAVES 99

namespace Access {

struct AccessGameDescription {
	ADGameDescription desc;

	int gameID;
	uint32 features;
};

uint32 AccessEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 AccessEngine::getGameFeatures() const {
	return _gameDescription->features;
}

uint32 AccessEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language AccessEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform AccessEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

} // End of namespace Access

static const PlainGameDescriptor AccessGames[] = {
	{"Access", "Access"},
	{"amazon", "Amazon: Guardians of Eden"},
	{0, 0}
};

#include "access/detection_tables.h"

class AccessMetaEngine : public AdvancedMetaEngine {
public:
	AccessMetaEngine() : AdvancedMetaEngine(Access::gameDescriptions, sizeof(Access::AccessGameDescription), AccessGames) {
		_maxScanDepth = 3;
	}

	virtual const char *getName() const {
		return "Access Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Access (c)";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool AccessMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail);
}

bool Access::AccessEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool AccessMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Access::AccessGameDescription *gd = (const Access::AccessGameDescription *)desc;
	if (gd) {
		*engine = new Access::AccessEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList AccessMetaEngine::listSaves(const char *target) const {
	SaveStateList saveList;

	return saveList;
}

int AccessMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void AccessMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor AccessMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	return SaveStateDescriptor();
}


#if PLUGIN_ENABLED_DYNAMIC(ACCESS)
	REGISTER_PLUGIN_DYNAMIC(ACCESS, PLUGIN_TYPE_ENGINE, AccessMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ACCESS, PLUGIN_TYPE_ENGINE, AccessMetaEngine);
#endif
