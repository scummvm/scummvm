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

#include "engines/advancedDetector.h"
#include "made/made.h"
#include "made/detection.h"


namespace Made {

uint32 MadeEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 MadeEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform MadeEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 MadeEngine::getVersion() const {
	return _gameDescription->version;
}

} // End of namespace Made

class MadeMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "made";
	}

    bool hasFeature(MetaEngineFeature f) const override;
	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

bool MadeMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		false;
}

bool Made::MadeEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher);
}

bool MadeMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Made::MadeGameDescription *gd = (const Made::MadeGameDescription *)desc;
	if (gd) {
		*engine = new Made::MadeEngine(syst, gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(MADE)
	REGISTER_PLUGIN_DYNAMIC(MADE, PLUGIN_TYPE_ENGINE, MadeMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MADE, PLUGIN_TYPE_ENGINE, MadeMetaEngine);
#endif
