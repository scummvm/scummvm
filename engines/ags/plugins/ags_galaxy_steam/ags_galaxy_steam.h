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

#ifndef AGS_PLUGINS_AGS_GALAXY_STEAM_AGS_GALAXY_STEAM_H
#define AGS_PLUGINS_AGS_GALAXY_STEAM_AGS_GALAXY_STEAM_H

#include "ags/plugins/ags_plugin.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/achievements.h"

namespace AGS3 {
namespace Plugins {
namespace AGSGalaxySteam {

class AGS2Client : public PluginBase {
	SCRIPT_HASH(AGS2Client)
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

public:
	AGS2Client() : PluginBase() {}
	virtual ~AGS2Client() {}

	void AGS_EngineStartup(IAGSEngine *engine) override;
};

class AGSGalaxy : public AGS2Client {
	SCRIPT_HASH_SUB(AGSGalaxy, AGS2Client)
public:
	AGSGalaxy() : AGS2Client() {}
	virtual ~AGSGalaxy() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *engine) override;
};

class AGSSteam : public AGS2Client {
	SCRIPT_HASH_SUB(AGSSteam, AGS2Client)
public:
	AGSSteam() : AGS2Client() {}
	virtual ~AGSSteam() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *engine) override;
};

} // namespace AGSGalaxySteam
} // namespace Plugins
} // namespace AGS3

#endif
