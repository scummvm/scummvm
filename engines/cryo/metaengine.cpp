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

#include "cryo/cryo.h"

namespace Cryo {

const char *CryoEngine::getGameId() const { return _gameDescription->gameId; }
bool CryoEngine::isDemo() const { return _gameDescription->flags & ADGF_DEMO; }
Common::Platform CryoEngine::getPlatform() const { return _gameDescription->platform; }

} // End of namespace Cryo

class CryoMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "cryo";
	}

    bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

bool CryoMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

Common::Error CryoMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Cryo::CryoEngine(syst, desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(CRYO)
	REGISTER_PLUGIN_DYNAMIC(CRYO, PLUGIN_TYPE_ENGINE, CryoMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CRYO, PLUGIN_TYPE_ENGINE, CryoMetaEngine);
#endif

