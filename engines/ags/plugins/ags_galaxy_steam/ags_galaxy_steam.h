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
	void IsAchievementAchieved(ScriptMethodParams &params);
	void SetAchievementAchieved(ScriptMethodParams &params);
	void ResetAchievement(ScriptMethodParams &params);
	void GetIntStat(ScriptMethodParams &params);
	void GetFloatStat(ScriptMethodParams &params);
	void GetAverageRateStat(ScriptMethodParams &params);
	void SetIntStat(ScriptMethodParams &params);
	void SetFloatStat(ScriptMethodParams &params);
	void UpdateAverageRateStat(ScriptMethodParams &params);
	void ResetStatsAndAchievements(ScriptMethodParams &params);
	void get_Initialized(ScriptMethodParams &params);
	void get_CurrentLeaderboardName(ScriptMethodParams &params);
	void RequestLeaderboard(ScriptMethodParams &params);
	void UploadScore(ScriptMethodParams &params);
	void geti_LeaderboardNames(ScriptMethodParams &params);
	void geti_LeaderboardScores(ScriptMethodParams &params);
	void get_LeaderboardCount(ScriptMethodParams &params);
	void GetUserName(ScriptMethodParams &params);
	void GetCurrentGameLanguage(ScriptMethodParams &params);
	void FindLeaderboard(ScriptMethodParams &params);
	void Initialize(ScriptMethodParams &params);

protected:
	void AGS_EngineStartup(IAGSEngine *engine) override;

public:
	AGS2Client();
};

class AGSGalaxy : public AGS2Client {
private:
	const char *AGS_GetPluginName();
	void AGS_EngineStartup(IAGSEngine *engine) override;

public:
	AGSGalaxy();
};

class AGSSteam : public AGS2Client {
private:
	const char *AGS_GetPluginName();

protected:
	void AGS_EngineStartup(IAGSEngine *engine) override;

public:
	AGSSteam();
};

} // namespace AGSGalaxySteam
} // namespace Plugins
} // namespace AGS3

#endif
