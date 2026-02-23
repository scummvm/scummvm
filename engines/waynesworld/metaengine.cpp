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

#include "waynesworld/waynesworld.h"

#include "engines/advancedDetector.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "waynesworld/detection.h"

class WaynesWorldMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "waynesworld";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

Common::Error WaynesWorldMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new WaynesWorld::WaynesWorldEngine(syst,desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(WAYNESWORLD)
	REGISTER_PLUGIN_DYNAMIC(WAYNESWORLD, PLUGIN_TYPE_ENGINE, WaynesWorldMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(WAYNESWORLD, PLUGIN_TYPE_ENGINE, WaynesWorldMetaEngine);
#endif
