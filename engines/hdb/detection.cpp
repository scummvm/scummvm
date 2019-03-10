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

#include "hdb/hdb.h"
#include "engines/advancedDetector.h"

static const PlainGameDescriptor hdbGames[] = {
	{"hdb", "Hyperspace Delivery Boy"},
	{0, 0}
};

namespace HDB {
static const ADGameDescription gameDescription[] = {
	{
		"hdb",
		//AD_ENTRY1s("GAME.BIN", 0, 41622),
		//TODO: Come up with an actual GameFileDescription
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};
}

class HDBMetaEngine : public AdvancedMetaEngine {
public:
	HDBMetaEngine() : AdvancedMetaEngine() {
		_singleId = "hdb";
	}

	virtual const char *getName() const {
		return "Hyperspace Delivery Boy";
	}

	virtual const char *getOriginalCopyright() const {
		return "";
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