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

#include "base/plugins.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "engines/advancedDetector.h"

#include "supernova/supernova.h"

static const PlainGameDescriptor supernovaGames[] = {
    {"msn1", "Mission Supernova 1"},
    {"msn2", "Mission Supernova 2"},
    {NULL, NULL}
};

namespace Supernova {
static const ADGameDescription gameDescriptions[] = {
    // Mission Supernova 1
    {
        "msn1",
        NULL,
        AD_ENTRY1s("msn.exe", "d11264516c529c7fc2ca81d8ba062d9e", 65536),
        Common::DE_DEU,
        Common::kPlatformDOS,
        ADGF_UNSTABLE,
        GUIO1(GUIO_NONE)
    },

    // Mission Supernova 2
    {
        "msn2",
        NULL,
        AD_ENTRY1s("ms2.exe", "e1b725393c3665f30efa96e2f46d589e", 82944),
        Common::DE_DEU,
        Common::kPlatformDOS,
        ADGF_UNSTABLE,
        GUIO1(GUIO_NONE)
    },

    AD_TABLE_END_MARKER
};
}

class SupernovaMetaEngine: public AdvancedMetaEngine {
public:
	SupernovaMetaEngine() : AdvancedMetaEngine(Supernova::gameDescriptions, sizeof(ADGameDescription), supernovaGames) {
//		_singleId = "supernova";
	}

	virtual const char *getName() const {
		return "Supernova Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Mission Supernova (c) 1994 Thomas and Steffen Dingel";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool SupernovaMetaEngine::hasFeature(MetaEngineFeature f) const {
	// STUB
	return false;
}

bool SupernovaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Supernova::SupernovaEngine(syst);
	}
	
	return desc != NULL;
}

#if PLUGIN_ENABLED_DYNAMIC(SUPERNOVA)
REGISTER_PLUGIN_DYNAMIC(SUPERNOVA, PLUGIN_TYPE_ENGINE, SupernovaMetaEngine);
#else
REGISTER_PLUGIN_STATIC(SUPERNOVA, PLUGIN_TYPE_ENGINE, SupernovaMetaEngine);
#endif
