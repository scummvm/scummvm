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

#include "hdb.h"

namespace HDB {
const char *HDBGame::getGameId() const { return _gameDescription->gameId; }
Common::Platform HDBGame::getPlatform() const { return _gameDescription->platform; }
} // End of namespace HDB

static const PlainGameDescriptor hdbGames[] = {
	{"hdb", "Hyperspace Delivery Boy!"},
	{0, 0}
};

namespace HDB {
static const ADGameDescription gameDescriptions[] = {
	{
		"hdb",
		"",
		AD_ENTRY1s("hyperdemo.mpc", "d8743b3b8be56486bcfb1398b2f2aad4", 13816461),
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},
	{
		"hdb",
		"",
		AD_ENTRY1s("hyperdemo.mpc", "f3bc878e179f00b8666a9846f3d9f9f5", 5236568),
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},
	// provided by sev
	{
		"hdb",
		"",
		AD_ENTRY1s("hyperdemo.mpc", "7bc533e8f1866931c884f1bc09353744", 13906865),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},
	AD_TABLE_END_MARKER
};
} // End of namespace HDB

class HDBMetaEngine : public AdvancedMetaEngine {
public:
	HDBMetaEngine() : AdvancedMetaEngine(HDB::gameDescriptions, sizeof(ADGameDescription), hdbGames) {
		_singleId = "hdb";
	}

	virtual const char *getName() const {
		return "Hyperspace Delivery Boy!";
	}

	virtual const char *getOriginalCopyright() const {
		return "Hyperspace Delivery Boy! (c) 2001 Monkeystone Games";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool HDBMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool HDBMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new HDB::HDBGame(syst, desc);
	}

	return desc != nullptr;
}

#if PLUGIN_ENABLED_DYNAMIC(HDB)
REGISTER_PLUGIN_DYNAMIC(HDB, PLUGIN_TYPE_ENGINE, HDBMetaEngine);
#else
REGISTER_PLUGIN_STATIC(HDB, PLUGIN_TYPE_ENGINE, HDBMetaEngine);
#endif
