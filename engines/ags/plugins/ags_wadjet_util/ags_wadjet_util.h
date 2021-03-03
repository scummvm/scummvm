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

#ifndef AGS_PLUGINS_AGS_WADGET_UTIL_AGS_WADGET_UTIL_H
#define AGS_PLUGINS_AGS_WADGET_UTIL_AGS_WADGET_UTIL_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWadgetUtil {

class AGSWadgetUtil : public PluginBase {
private:
	static IAGSEngine *_engine;
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *lpEngine);

private:
	static NumberPtr IsOnPhone(const ScriptMethodParams &params);
	static void FakeKeypress(const ScriptMethodParams &params);
	static void IosSetAchievementValue(const ScriptMethodParams &params);
	static NumberPtr IosGetAchievementValue(const ScriptMethodParams &params);
	static void IosShowAchievements(const ScriptMethodParams & params);
	static void IosResetAchievements(const ScriptMethodParams &params);
	static NumberPtr MobileGetAchievement(const ScriptMethodParams &params);
	static NumberPtr MobileSetAchievement(const ScriptMethodParams &params);
	static void MobileShowAchievements(const ScriptMethodParams &params);
	static void MobileResetAchievements(const ScriptMethodParams &params);

public:
	AGSWadgetUtil();
};

} // namespace AGSWadgetUtil
} // namespace Plugins
} // namespace AGS3

#endif
