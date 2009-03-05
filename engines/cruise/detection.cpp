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
 * $URL$
 * $Id$
 *
 */



#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "cruise/cruise.h"

namespace Cruise {

struct CRUISEGameDescription {
	ADGameDescription desc;

	int gameType;
	uint32 features;
};

int CruiseEngine::getGameType() const {
	return _gameDescription->gameType;
}
uint32 CruiseEngine::getFeatures() const {
	return _gameDescription->features;
}
Common::Language CruiseEngine::getLanguage() const {
	return _gameDescription->desc.language;
}
Common::Platform CruiseEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

}

static const PlainGameDescriptor cruiseGames[] = {
	{"cruise", "Cinematique evo.2 engine game"},
	{"cruise", "Cruise for a corpse"},
	{0, 0}
};

static const ADObsoleteGameID obsoleteGameIDsTable[] = {
	{"cruise", "cruise", Common::kPlatformUnknown},
	{0, 0, Common::kPlatformUnknown}
};

namespace Cruise {

static const CRUISEGameDescription gameDescriptions[] = {
	{
		{
			"cruise",
			"16 colors",
			AD_ENTRY1("D1", "41a7a4d426dbd048eb369cfee4bb2717"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "a90d2b9ead6b4d812cd14268672cf178"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "e258865807ea31b2d523340e6f0a606b"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "f2a26522d49983c4ae32bcccbb801b02"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "e19a4ab2e24a69087e4ea994a5506231"),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "9a302ada55600d96061fda1d63a6ccda"),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS
		},
		GType_CRUISE,
		0,
	},
	{AD_TABLE_END_MARKER, 0, 0}
};

}

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Cruise::gameDescriptions,
	// Size of that superset structure
	sizeof(Cruise::CRUISEGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	cruiseGames,
	// Structure for autoupgrading obsolete targets
	obsoleteGameIDsTable,
	// Name of single gameid (optional)
	"cruise",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0
};

class CruiseMetaEngine : public AdvancedMetaEngine {
public:
	CruiseMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Cinematique evo 2 engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Cruise for a Corpse (C) Delphine Software";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool CruiseMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Cruise::CRUISEGameDescription *gd = (const Cruise::CRUISEGameDescription *)desc;
	if (gd) {
		*engine = new Cruise::CruiseEngine(syst, gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(CRUISE)
REGISTER_PLUGIN_DYNAMIC(CRUISE, PLUGIN_TYPE_ENGINE, CruiseMetaEngine);
#else
REGISTER_PLUGIN_STATIC(CRUISE, PLUGIN_TYPE_ENGINE, CruiseMetaEngine);
#endif
