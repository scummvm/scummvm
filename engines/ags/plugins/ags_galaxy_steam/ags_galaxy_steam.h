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

namespace AGS3 {
namespace Plugins {
namespace AGSGalaxySteam {

class AGS2Client : public PluginBase {
protected:
	static int IsAchievementAchieved(const ScriptMethodParams &params);
	static int SetAchievementAchieved(const ScriptMethodParams &params);
	static int ResetAchievement(const ScriptMethodParams &params);
	static int GetIntStat(const ScriptMethodParams &params);
	static int GetFloatStat(const ScriptMethodParams &params);
	static int GetAverageRateStat(const ScriptMethodParams &params);
	static int SetIntStat(const ScriptMethodParams &params);
	static int SetFloatStat(const ScriptMethodParams &params);
	static int UpdateAverageRateStat(const ScriptMethodParams &params);
	static void ResetStatsAndAchievements(const ScriptMethodParams &params);
	static int get_Initialized(const ScriptMethodParams &params);
	static void get_CurrentLeaderboardName(const ScriptMethodParams &params);
	static void RequestLeaderboard(const ScriptMethodParams &params);
	static int UploadScore(const ScriptMethodParams &params);
	static void geti_LeaderboardNames(const ScriptMethodParams &params);
	static int geti_LeaderboardScores(const ScriptMethodParams &params);
	static int get_LeaderboardCount(const ScriptMethodParams &params);
	static void GetUserName(const ScriptMethodParams &params);
	static void GetCurrentGameLanguage(const ScriptMethodParams &params);
	static void FindLeaderboard(const ScriptMethodParams &params);
	static int Initialize(const ScriptMethodParams &params);

protected:
	static void AGS_EngineStartup(IAGSEngine *engine);

public:
	AGS2Client();
};

class AGSGalaxy : public AGS2Client {
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);

	static int IsAchievementAchieved(const ScriptMethodParams &params);
	static int SetAchievementAchieved(const ScriptMethodParams &params);
	static int ResetAchievement(const ScriptMethodParams &params);
	static int GetIntStat(const ScriptMethodParams &params);
	static int GetFloatStat(const ScriptMethodParams &params);
	static int GetAverageRateStat(const ScriptMethodParams &params);
	static int SetIntStat(const ScriptMethodParams &params);
	static int SetFloatStat(const ScriptMethodParams &params);
	static int UpdateAverageRateStat(const ScriptMethodParams &params);
	static void ResetStatsAndAchievements(const ScriptMethodParams &params);
	static int get_Initialized(const ScriptMethodParams &params);
	static void get_CurrentLeaderboardName(const ScriptMethodParams &params);
	static void RequestLeaderboard(const ScriptMethodParams &params);
	static int UploadScore(const ScriptMethodParams &params);
	static void geti_LeaderboardNames(const ScriptMethodParams &params);
	static int geti_LeaderboardScores(const ScriptMethodParams &params);
	static int get_LeaderboardCount(const ScriptMethodParams &params);
	static void GetUserName(const ScriptMethodParams &params);
	static void GetCurrentGameLanguage(const ScriptMethodParams &params);
	static int Initialize(const ScriptMethodParams &params);

public:
	AGSGalaxy();
};

class AGSSteam : public AGS2Client {
private:
	static const char *AGS_GetPluginName();

protected:
	static void AGS_EngineStartup(IAGSEngine *engine);

	static int IsAchievementAchieved(const ScriptMethodParams &params);
	static int SetAchievementAchieved(const ScriptMethodParams &params);
	static int ResetAchievement(const ScriptMethodParams &params);
	static int GetIntStat(const ScriptMethodParams &params);
	static int GetFloatStat(const ScriptMethodParams &params);
	static int GetAverageRateStat(const ScriptMethodParams &params);
	static int SetIntStat(const ScriptMethodParams &params);
	static int SetFloatStat(const ScriptMethodParams &params);
	static int UpdateAverageRateStat(const ScriptMethodParams &params);
	static void ResetStatsAndAchievements(const ScriptMethodParams &params);
	static int get_Initialized(const ScriptMethodParams &params);
	static void get_CurrentLeaderboardName(const ScriptMethodParams &params);
	static void RequestLeaderboard(const ScriptMethodParams &params);
	static int UploadScore(const ScriptMethodParams &params);
	static void geti_LeaderboardNames(const ScriptMethodParams &params);
	static int geti_LeaderboardScores(const ScriptMethodParams &params);
	static int get_LeaderboardCount(const ScriptMethodParams &params);
	static void GetUserName(const ScriptMethodParams &params);
	static void GetCurrentGameLanguage(const ScriptMethodParams &params);
	static int FindLeaderboard(const ScriptMethodParams &params);

public:
	AGSSteam();
};

} // namespace AGSGalaxySteam
} // namespace Plugins
} // namespace AGS3

#endif
