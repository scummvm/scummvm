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

#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "prince/prince.h"

namespace Prince {

struct PrinceGameDescription {
	ADGameDescription desc;

	int gameType;
};

int PrinceEngine::getGameType() const {
	return _gameDescription->gameType;
}

const char *PrinceEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}

uint32 PrinceEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language PrinceEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

}

static const PlainGameDescriptor princeGames[] = {
	{"prince", "Prince Game"},
	{0, 0}
};

namespace Prince {

static const PrinceGameDescription gameDescriptions[] = {
	
	// German
	{
		{
			"prince",
			"Galador",
			AD_ENTRY1s("databank.ptc", "5fa03833177331214ec1354761b1d2ee", 3565031),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		0
	},
	// Polish
	{
		{
			"prince",
			"Ksiaze i Tchorz",
			AD_ENTRY1s("databank.ptc", "48ec9806bda9d152acbea8ce31c93c49", 3435298),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		1
	},


	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Prince

using namespace Prince;

// we match from data too, to stop detection from a non-top-level directory
const static char *directoryGlobs[] = {
	"all",
	0
};

class PrinceMetaEngine : public AdvancedMetaEngine {
public:
	PrinceMetaEngine() : AdvancedMetaEngine(Prince::gameDescriptions, sizeof(Prince::PrinceGameDescription), princeGames) {
		_singleid = "prince";
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getName() const {
		return "Prince Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Copyright (C)";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
};

bool PrinceMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	using namespace Prince;
	const PrinceGameDescription *gd = (const PrinceGameDescription *)desc;
	if (gd) {
		*engine = new PrinceEngine(syst, gd);
	}
	return gd != 0;
}

bool PrinceMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Prince::PrinceEngine::hasFeature(EngineFeature f) const {
	return false;//(f == kSupportsRTL);
}

#if PLUGIN_ENABLED_DYNAMIC(PRINCE)
REGISTER_PLUGIN_DYNAMIC(PRINCE, PLUGIN_TYPE_ENGINE, PrinceMetaEngine);
#else
REGISTER_PLUGIN_STATIC(PRINCE, PLUGIN_TYPE_ENGINE, PrinceMetaEngine);
#endif

/* vim: set tabstop=4 noexpandtab: */
