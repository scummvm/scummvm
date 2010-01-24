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

#include "engines/stark/stark.h"


namespace Stark {

static const PlainGameDescriptor starkGames[] = {
	{"stark", "Stark Game"},
	{"tlj", "The Longest Journey"},
	{0, 0}
};

static const ADGameDescription gameDescriptions[] = {
	// The Longest Journey
	// English 4CD
	{
		"tlj", "",
		AD_ENTRY1("x.xarc", "a0559457126caadab0cadac02d35f26f"),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},

	// The Longest Journey
	// English DVD
	{
		"tlj", "DVD",
		AD_ENTRY1("x.xarc", "de8327850d7bba90b690b141eaa23f61"),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_DVD,
		Common::GUIO_NONE
	},
	
	// The Longest Journey
	// English Demo
	{
		"tlj", "",
		AD_ENTRY1("x.xarc", "97abc1bb9239dee4c208e533f3c97e1c"),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		Common::GUIO_NONE
	},
	
	// The Longest Journey
	// English DVD Demo
	{
		"tlj", "DVD",
		AD_ENTRY1("x.xarc", "61093bcd499b386ed5c0345c52f48909"),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | GF_DVD,
		Common::GUIO_NONE
	},

	// The Longest Journey
	// Spanish 4CD
	{
		"tlj", "",
		AD_ENTRY1s("x.xarc", "a0559457126caadab0cadac02d35f26f", 3032),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},

	AD_TABLE_END_MARKER
};

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)gameDescriptions,
	// Size of that superset structure
	sizeof(ADGameDescription),
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
	0,
	// GUI Options
	Common::GUIO_NOMIDI
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
	if (desc) 
		*engine = new StarkEngine(syst, desc);
	
	return desc != 0;
}

} // End of namespace Stark

#if PLUGIN_ENABLED_DYNAMIC(STARK)
	REGISTER_PLUGIN_DYNAMIC(STARK, PLUGIN_TYPE_ENGINE, Stark::StarkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(STARK, PLUGIN_TYPE_ENGINE, Stark::StarkMetaEngine);
#endif
