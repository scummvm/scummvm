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

#ifndef AGS_PLUGINS_AGS_GALAXY_STEAM_AGS_GALAXY_STEAM_H
#define AGS_PLUGINS_AGS_GALAXY_STEAM_AGS_GALAXY_STEAM_H

#include "ags/plugins/plugin_base.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/achievements.h"

namespace AGS3 {
namespace Plugins {
namespace AGSGalaxySteam {

class AGS2Client : public PluginBase {
protected:
	static void IsAchievementAchieved(ScriptMethodParams &params);
	static void SetAchievementAchieved(ScriptMethodParams &params);
	static void ResetAchievement(ScriptMethodParams &params);
	static void GetIntStat(ScriptMethodParams &params);
	static void GetFloatStat(ScriptMethodParams &params);
	static void GetAverageRateStat(ScriptMethodParams &params);
	static void SetIntStat(ScriptMethodParams &params);
	static void SetFloatStat(ScriptMethodParams &params);
	static void UpdateAverageRateStat(ScriptMethodParams &params);
	static void ResetStatsAndAchievements(ScriptMethodParams &params);
	static void get_Initialized(ScriptMethodParams &params);
	static void get_CurrentLeaderboardName(ScriptMethodParams &params);
	static void RequestLeaderboard(ScriptMethodParams &params);
	static void UploadScore(ScriptMethodParams &params);
	static void geti_LeaderboardNames(ScriptMethodParams &params);
	static void geti_LeaderboardScores(ScriptMethodParams &params);
	static void get_LeaderboardCount(ScriptMethodParams &params);
	static void GetUserName(ScriptMethodParams &params);
	static void GetCurrentGameLanguage(ScriptMethodParams &params);
	static void FindLeaderboard(ScriptMethodParams &params);
	static void Initialize(ScriptMethodParams &params);

protected:
	static void AGS_EngineStartup(IAGSEngine *engine);

public:
	AGS2Client();
};

class AGSGalaxy : public AGS2Client {
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);

	static void IsAchievementAchieved(ScriptMethodParams &params);
	static void SetAchievementAchieved(ScriptMethodParams &params);
	static void ResetAchievement(ScriptMethodParams &params);
	static void GetIntStat(ScriptMethodParams &params);
	static void GetFloatStat(ScriptMethodParams &params);
	static void GetAverageRateStat(ScriptMethodParams &params);
	static void SetIntStat(ScriptMethodParams &params);
	static void SetFloatStat(ScriptMethodParams &params);
	static void UpdateAverageRateStat(ScriptMethodParams &params);
	static void ResetStatsAndAchievements(ScriptMethodParams &params);
	static void get_Initialized(ScriptMethodParams &params);
	static void get_CurrentLeaderboardName(ScriptMethodParams &params);
	static void RequestLeaderboard(ScriptMethodParams &params);
	static void UploadScore(ScriptMethodParams &params);
	static void geti_LeaderboardNames(ScriptMethodParams &params);
	static void geti_LeaderboardScores(ScriptMethodParams &params);
	static void get_LeaderboardCount(ScriptMethodParams &params);
	static void GetUserName(ScriptMethodParams &params);
	static void GetCurrentGameLanguage(ScriptMethodParams &params);
	static void Initialize(ScriptMethodParams &params);

public:
	AGSGalaxy();
};

class AGSSteam : public AGS2Client {
private:
	static const char *AGS_GetPluginName();

protected:
	static void AGS_EngineStartup(IAGSEngine *engine);

	static void IsAchievementAchieved(ScriptMethodParams &params);
	static void SetAchievementAchieved(ScriptMethodParams &params);
	static void ResetAchievement(ScriptMethodParams &params);
	static void GetIntStat(ScriptMethodParams &params);
	static void GetFloatStat(ScriptMethodParams &params);
	static void GetAverageRateStat(ScriptMethodParams &params);
	static void SetIntStat(ScriptMethodParams &params);
	static void SetFloatStat(ScriptMethodParams &params);
	static void UpdateAverageRateStat(ScriptMethodParams &params);
	static void ResetStatsAndAchievements(ScriptMethodParams &params);
	static void get_Initialized(ScriptMethodParams &params);
	static void get_CurrentLeaderboardName(ScriptMethodParams &params);
	static void RequestLeaderboard(ScriptMethodParams &params);
	static void UploadScore(ScriptMethodParams &params);
	static void geti_LeaderboardNames(ScriptMethodParams &params);
	static void geti_LeaderboardScores(ScriptMethodParams &params);
	static void get_LeaderboardCount(ScriptMethodParams &params);
	static void GetUserName(ScriptMethodParams &params);
	static void GetCurrentGameLanguage(ScriptMethodParams &params);
	static void FindLeaderboard(ScriptMethodParams &params);

public:
	AGSSteam();
};

} // namespace AGSGalaxySteam
} // namespace Plugins
} // namespace AGS3

#endif
