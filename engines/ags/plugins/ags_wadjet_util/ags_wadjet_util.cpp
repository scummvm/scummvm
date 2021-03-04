/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_wadjet_util/ags_wadjet_util.h"
#include "ags/shared/core/platform.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWadgetUtil {

IAGSEngine *AGSWadgetUtil::_engine;

AGSWadgetUtil::AGSWadgetUtil() : PluginBase() {
	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSWadgetUtil::AGS_GetPluginName() {
	return "AGSWadgetUtil";
}

void AGSWadgetUtil::AGS_EngineStartup(IAGSEngine *engine) {
	_engine = engine;

	// Register functions
	SCRIPT_METHOD(IsOnPhone);
	SCRIPT_METHOD(FakeKeypress);
	SCRIPT_METHOD(IosSetAchievementValue);
	SCRIPT_METHOD(IosGetAchievementValue);
	SCRIPT_METHOD(IosShowAchievements);
	SCRIPT_METHOD(IosResetAchievements);
	SCRIPT_METHOD(MobileGetAchievement);
	SCRIPT_METHOD(MobileSetAchievement);
	SCRIPT_METHOD(MobileShowAchievements);
	SCRIPT_METHOD(MobileResetAchievements);
}

void AGSWadgetUtil::IsOnPhone(ScriptMethodParams &params) {
	params._result = false;
}

void AGSWadgetUtil::FakeKeypress(ScriptMethodParams &params) {
}

void AGSWadgetUtil::IosSetAchievementValue(ScriptMethodParams &params) {
}

void AGSWadgetUtil::IosGetAchievementValue(ScriptMethodParams &params) {
	params._result = -1;
}

void AGSWadgetUtil::IosShowAchievements(ScriptMethodParams &params) {
}

void AGSWadgetUtil::IosResetAchievements(ScriptMethodParams &params) {
}

void AGSWadgetUtil::MobileGetAchievement(ScriptMethodParams &params) {
	params._result = NumberPtr();
}

void AGSWadgetUtil::MobileSetAchievement(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSWadgetUtil::MobileShowAchievements(ScriptMethodParams &params) {
}

void AGSWadgetUtil::MobileResetAchievements(ScriptMethodParams &params) {
}

} // namespace AGSWadgetUtil
} // namespace Plugins
} // namespace AGS3
