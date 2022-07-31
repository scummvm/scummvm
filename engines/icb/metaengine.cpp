/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/icb/icb.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/config-manager.h"

namespace ICB {

class IcbMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "icb";
	}
	bool hasFeature(MetaEngineFeature f) const override { return false; }

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error IcbMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const IcbGameDescription *gd = (const IcbGameDescription *)desc;
	*engine = new IcbEngine(syst, gd);
	return Common::kNoError;
}

Common::KeymapArray IcbMetaEngine::initKeymaps(const char *target) const {
	Common::String gameId = ConfMan.get("gameid", target);

	if (gameId == "icb") {
		return ICB::IcbEngine::initKeymapsIcb(target);
	} else if (gameId == "eldorado") {
		return ICB::IcbEngine::initKeymapsEldorado(target);
	}

	return AdvancedMetaEngine::initKeymaps(target);
}

} // End of namespace ICB

#if PLUGIN_ENABLED_DYNAMIC(ICB)
	REGISTER_PLUGIN_DYNAMIC(ICB, PLUGIN_TYPE_ENGINE, ICB::IcbMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ICB, PLUGIN_TYPE_ENGINE, ICB::IcbMetaEngine);
#endif
