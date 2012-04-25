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

#include "engines/advancedDetector.h"
#include "common/system.h"

#include "tony/tony.h"


namespace Tony {

struct TonyGameDescription {
	ADGameDescription desc;
};

uint32 TonyEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language TonyEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

}

static const PlainGameDescriptor tonyGames[] = {
	{"tony", "Tony Tough and the Night of Roasted Moths"},
	{0, 0}
};

#include "tony/detection_tables.h"

class TonyMetaEngine : public AdvancedMetaEngine {
public:
	TonyMetaEngine() : AdvancedMetaEngine(Tony::gameDescriptions, sizeof(Tony::TonyGameDescription), tonyGames) {
	}

	virtual const char *getName() const {
		return "Tony Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Tony Engine (C) Protonic Interactive";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool TonyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Tony::TonyEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

bool TonyMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Tony::TonyGameDescription *gd = (const Tony::TonyGameDescription *)desc;
	if (gd) {
		*engine = new Tony::TonyEngine(syst, gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(TONY)
	REGISTER_PLUGIN_DYNAMIC(TONY, PLUGIN_TYPE_ENGINE, TonyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TONY, PLUGIN_TYPE_ENGINE, TonyMetaEngine);
#endif
