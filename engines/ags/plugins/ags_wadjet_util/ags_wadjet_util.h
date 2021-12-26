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

#ifndef AGS_PLUGINS_AGS_WADJET_UTIL_AGS_WADJET_UTIL_H
#define AGS_PLUGINS_AGS_WADJET_UTIL_AGS_WADJET_UTIL_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWadjetUtil {

class AGSWadjetUtil : public PluginBase {
	SCRIPT_HASH(AGSWadjetUtil)
private:
	void IsOnPhone(ScriptMethodParams &params);
	void FakeKeypress(ScriptMethodParams &params);
	void IosSetAchievementValue(ScriptMethodParams &params);
	void IosGetAchievementValue(ScriptMethodParams &params);
	void IosShowAchievements(ScriptMethodParams &params);
	void IosResetAchievements(ScriptMethodParams &params);
	void MobileGetAchievement(ScriptMethodParams &params);
	void MobileSetAchievement(ScriptMethodParams &params);
	void MobileShowAchievements(ScriptMethodParams &params);
	void MobileResetAchievements(ScriptMethodParams &params);

public:
	AGSWadjetUtil() : PluginBase() {}
	virtual ~AGSWadjetUtil() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *lpEngine) override;
};

} // namespace AGSWadjetUtil
} // namespace Plugins
} // namespace AGS3

#endif
