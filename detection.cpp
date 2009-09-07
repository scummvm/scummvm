/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "engines/stark/stark.h"


namespace Stark {

struct StarkGameDescription {
	ADGameDescription desc;

	uint8 gameType;
	uint32 features;
	uint16 version;
};

uint32 StarkEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform StarkEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 StarkEngine::getVersion() const {
	return _gameDescription->version;
}

int StarkEngine::getGameID() const {
	return _gameDescription->gameType;
}

Common::Language StarkEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of Namespace Stark

static const PlainGameDescriptor starkGames[] = {
	{"stark", "Stark Game"},
	{"tlj", "The Longest Journey"},
	{0, 0}
};


namespace Stark {

static const StarkGameDescription gameDescriptions[] = {
	// The Longest Journey
	// English 4CD
	{
		{
			"tlj",
			"",
			AD_ENTRY1("x.xarc", "a0559457126caadab0cadac02d35f26f"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS
		},
		GID_TLJ,
		0,
		0,
	},

	// The Longest Journey
	// English DVD
	/*{
		{
			"tlj",
			"DVD",
			AD_ENTRY1("x.xarc", "de8327850d7bba90b690b141eaa23f61"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS
		},
		GID_TLJ,
		GF_DVD,
		0,
	},
	
	// The Longest Journey
	// English Demo
	{
		{
			"tlj",
			"",
			AD_ENTRY1("x.xarc", ""),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS
		},
		GID_TLJ,
		GF_DEMO,
		0,
	},*/

	{ AD_TABLE_END_MARKER, 0, 0, 0 }
};

} // End of namespace Stark

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Stark::gameDescriptions,
	// Size of that superset structure
	sizeof(Stark::StarkGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	starkGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"stark",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0
};

class StarkMetaEngine : public AdvancedMetaEngine {
public:
	StarkMetaEngine() : AdvancedMetaEngine(detectionParams) {}
	
	virtual const char *getName() const {
		return "Stark Engine";
	}
	
	virtual const char *getOriginalCopyright() const { 
		return "(C) Funcom";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool StarkMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Stark::StarkGameDescription *gd = (const Stark::StarkGameDescription *)desc;
	
	if (gd) 
		*engine = new Stark::StarkEngine(syst, gd);
	
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(STARK)
	REGISTER_PLUGIN_DYNAMIC(STARK, PLUGIN_TYPE_ENGINE, StarkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(STARK, PLUGIN_TYPE_ENGINE, StarkMetaEngine);
#endif
