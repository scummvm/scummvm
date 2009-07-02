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

#include "draci/draci.h"

#include "base/plugins.h"
#include "engines/metaengine.h"

static const PlainGameDescriptor draciGames[] = {
	{ "draci", "Draci Historie" },
	{ 0, 0 }
};

namespace Draci {

using Common::GUIO_NONE;

const ADGameDescription gameDescriptions[] = {
	
	{
		"draci",
		0,
		AD_ENTRY1s("INIT.DFW", "b890a5aeebaf16af39219cba2416b0a3", 906),
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},
	
	{
		"draci",
		0,
		AD_ENTRY1s("INIT.DFW", "9921c8f0045679a8f37eca8d41c5ec02", 906),
		Common::CZ_CZE,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	{
		"draci",
		0,
		AD_ENTRY1s("INIT.DFW", "76b9b78a8a8809a240acc395df4d0715", 906),
		Common::PL_POL,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Draci

const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Draci::gameDescriptions,
	// Size of that superset structure
	sizeof(ADGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	draciGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"draci",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0,
	// Global GUI options
	Common::GUIO_NONE
};

class DraciMetaEngine : public AdvancedMetaEngine {
public:
	DraciMetaEngine() : AdvancedMetaEngine(detectionParams) {}
	
	virtual const char *getName() const {
		return "Draci Historie Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Copyright (C) 1995 NoSense";
	}
	
	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool DraciMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Draci::DraciEngine::hasFeature(EngineFeature f) const {
	return false;
}

bool DraciMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Draci::DraciEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(DRACI)
	REGISTER_PLUGIN_DYNAMIC(DRACI, PLUGIN_TYPE_ENGINE, DraciMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DRACI, PLUGIN_TYPE_ENGINE, DraciMetaEngine);
#endif
