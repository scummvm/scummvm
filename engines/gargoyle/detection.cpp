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

#include "gargoyle/gargoyle.h"

#include "base/plugins.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"

#define MAX_SAVES 99

namespace Gargoyle {

struct GargoyleGameDescription {
	ADGameDescription desc;
	InterpreterType interpType;
};

uint32 GargoyleEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

bool GargoyleEngine::isDemo() const {
	return (bool)(_gameDescription->desc.flags & ADGF_DEMO);
}

Common::Language GargoyleEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

InterpreterType GargoyleEngine::getInterpreterType() const {
	return _gameDescription->interpType;
}

} // End of namespace Gargoyle

static const PlainGameDescriptor GargoyleGames[] = {
	{"Gargoyle", "Gargoyle Games"},
	{0, 0}
};

#include "gargoyle/detection_tables.h"

class GargoyleMetaEngine : public AdvancedMetaEngine {
public:
	GargoyleMetaEngine() : AdvancedMetaEngine(Gargoyle::gameDescriptions, sizeof(Gargoyle::GargoyleGameDescription), GargoyleGames) {
		_maxScanDepth = 3;
	}

	virtual const char *getName() const {
		return "Gargoyle Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Gargoyle Engine (c)";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool GargoyleMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail);
}

bool Gargoyle::GargoyleEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool GargoyleMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Gargoyle::GargoyleGameDescription *gd = (const Gargoyle::GargoyleGameDescription *)desc;
	*engine = new Gargoyle::GargoyleEngine(syst, gd);

	return gd != 0;
}

SaveStateList GargoyleMetaEngine::listSaves(const char *target) const {
	SaveStateList saveList;
	return saveList;
}

int GargoyleMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void GargoyleMetaEngine::removeSaveState(const char *target, int slot) const {
}

SaveStateDescriptor GargoyleMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	return SaveStateDescriptor();
}


#if PLUGIN_ENABLED_DYNAMIC(GARGOYLE)
	REGISTER_PLUGIN_DYNAMIC(Gargoyle, PLUGIN_TYPE_ENGINE, GargoyleMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GARGOYLE, PLUGIN_TYPE_ENGINE, GargoyleMetaEngine);
#endif
