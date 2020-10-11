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

#include "engines/advancedDetector.h"
#include "common/file.h"

static const PlainGameDescriptor kingdomGames[] = {
	{"kingdom", "Kingdom: The Far Reaches"},
	{0, 0}
};

namespace Kingdom {

static const ADGameDescription gameDescriptions[] = {
	// Kingdom PC DOS Demo version, provided by Strangerke
	{
		"kingdom",
		"Demo",
		AD_ENTRY1s("KINGDEMO.EXE", "2ce478fc13086d0ddb02a2867ba307dc", 167154),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},

	// Kingdom PC DOS version v1.26 EN, provided by Strangerke
	// Also detects the GOG.COM version
	{
		"kingdom",
		0,
		AD_ENTRY1s("KINGDOM.EXE", "64d3e03b963396ced402f3dc958765c0", 199693),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Kingdom 3DO, provided by Strangerke
    {
        "kingdom",
        0,
        AD_ENTRY1s("launchme", "60d2c64e3cb3e22859c4fadbc121b0db", 183452),
        Common::EN_ANY,
        Common::kPlatform3DO,
        ADGF_UNSTABLE,
        GUIO1(GUIO_NONE)
    },

	AD_TABLE_END_MARKER
};

} // End of namespace Kingdom

class KingdomMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	KingdomMetaEngineDetection() : AdvancedMetaEngineDetection(Kingdom::gameDescriptions, sizeof(ADGameDescription), kingdomGames) {
	}

	const char *getEngineId() const override {
		return "kingdom";
	}

	virtual const char *getName() const override {
		return "Kingdom: The Far Reaches' Engine";
	}

	virtual const char *getOriginalCopyright() const override {
		return "Kingdom: The far Reaches (C) 1995 Virtual Image Productions";
	}
};

REGISTER_PLUGIN_STATIC(KINGDOM_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, KingdomMetaEngineDetection);
