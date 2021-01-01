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

#include "common/system.h"

#include "engines/advancedDetector.h"

#include "testbed/testbed.h"
#include "testbed/testsuite.h"

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
		Common::AchievementDescription testSuiteFinalAchievement = {"EVERYTHINGWORKS", true, "Everything works!", "Completed all available testsuites"};
		result.descriptions.push_back(testSuiteFinalAchievement);

		Common::Array<Testbed::Testsuite *> testsuiteList;
		Testbed::TestbedEngine::pushTestsuites(testsuiteList);
		for (Common::Array<Testbed::Testsuite *>::const_iterator i = testsuiteList.begin(); i != testsuiteList.end(); ++i) {
			Common::AchievementDescription it = {(*i)->getName(), false, (*i)->getDescription(), 0};
			result.descriptions.push_back(it);
			delete (*i);
		}

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
