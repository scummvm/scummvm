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

#include "base/plugins.h"

#include "common/system.h"

#include "engines/advancedDetector.h"

#include "testbed/testbed.h"

class TestbedMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "testbed";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription * /* desc */) const override {
		*engine = new Testbed::TestbedEngine(syst);
		return Common::kNoError;
	}

	const Common::AchievementsInfo getAchievementsInfo(const Common::String &target) const override {
		Common::AchievementsInfo result;
		result.platform = Common::UNK_ACHIEVEMENTS;
		result.appId = "testbed";
		return result;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return false;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(TESTBED)
	REGISTER_PLUGIN_DYNAMIC(TESTBED, PLUGIN_TYPE_ENGINE, TestbedMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TESTBED, PLUGIN_TYPE_ENGINE, TestbedMetaEngine);
#endif
