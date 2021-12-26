/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_wadjet_util/ags_wadjet_util.h"
#include "ags/shared/core/platform.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWadjetUtil {

const char *AGSWadjetUtil::AGS_GetPluginName() {
	return "AGSWadjetUtil";
}

void AGSWadjetUtil::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	// Register functions
	SCRIPT_METHOD(IsOnPhone, AGSWadjetUtil::IsOnPhone);
	SCRIPT_METHOD(FakeKeypress, AGSWadjetUtil::FakeKeypress);
	SCRIPT_METHOD(IosSetAchievementValue, AGSWadjetUtil::IosSetAchievementValue);
	SCRIPT_METHOD(IosGetAchievementValue, AGSWadjetUtil::IosGetAchievementValue);
	SCRIPT_METHOD(IosShowAchievements, AGSWadjetUtil::IosShowAchievements);
	SCRIPT_METHOD(IosResetAchievements, AGSWadjetUtil::IosResetAchievements);
	SCRIPT_METHOD(MobileGetAchievement, AGSWadjetUtil::MobileGetAchievement);
	SCRIPT_METHOD(MobileSetAchievement, AGSWadjetUtil::MobileSetAchievement);
	SCRIPT_METHOD(MobileShowAchievements, AGSWadjetUtil::MobileShowAchievements);
	SCRIPT_METHOD(MobileResetAchievements, AGSWadjetUtil::MobileResetAchievements);
}

void AGSWadjetUtil::IsOnPhone(ScriptMethodParams &params) {
	params._result = false;
}

void AGSWadjetUtil::FakeKeypress(ScriptMethodParams &params) {
}

void AGSWadjetUtil::IosSetAchievementValue(ScriptMethodParams &params) {
}

void AGSWadjetUtil::IosGetAchievementValue(ScriptMethodParams &params) {
	params._result = -1;
}

void AGSWadjetUtil::IosShowAchievements(ScriptMethodParams &params) {
}

void AGSWadjetUtil::IosResetAchievements(ScriptMethodParams &params) {
}

void AGSWadjetUtil::MobileGetAchievement(ScriptMethodParams &params) {
	params._result = NumberPtr();
}

void AGSWadjetUtil::MobileSetAchievement(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSWadjetUtil::MobileShowAchievements(ScriptMethodParams &params) {
}

void AGSWadjetUtil::MobileResetAchievements(ScriptMethodParams &params) {
}

} // namespace AGSWadjetUtil
} // namespace Plugins
} // namespace AGS3
