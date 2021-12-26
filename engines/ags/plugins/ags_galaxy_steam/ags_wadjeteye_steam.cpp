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

#include "ags/plugins/ags_galaxy_steam/ags_wadjeteye_steam.h"

namespace AGS3 {
namespace Plugins {
namespace AGSGalaxySteam {

void AGSWadjetEyeSteam::AddAchievement(ScriptMethodParams &params) {
	warning("AGSWadjetEyeSteam::AddAchievement() is not implemented");
	params._result = 0;
}

void AGSWadjetEyeSteam::AddStat(ScriptMethodParams &params) {
	warning("AGSWadjetEyeSteam::AddStat() is not implemented");
	params._result = 0;
}

void AGSWadjetEyeSteam::AGS_EngineStartup(IAGSEngine *engine) {
	AGSSteam::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Steam::AddAchievement^1, AGSWadjetEyeSteam::AddAchievement);
	SCRIPT_METHOD(Steam::AddStat^2, AGSWadjetEyeSteam::AddStat);
	SCRIPT_METHOD(Steam::IsAchievementAchieved^1, AGSWadjetEyeSteam::IsAchievementAchieved);
	SCRIPT_METHOD(Steam::SetAchievementAchieved^1, AGSWadjetEyeSteam::SetAchievementAchieved);
	SCRIPT_METHOD(Steam::ResetAchievement^1, AGSWadjetEyeSteam::ResetAchievement);
	SCRIPT_METHOD(Steam::GetIntStat^1, AGSWadjetEyeSteam::GetIntStat);
	SCRIPT_METHOD(Steam::GetFloatStat^1, AGSWadjetEyeSteam::GetFloatStat);
	SCRIPT_METHOD(Steam::GetAverageRateStat^1, AGSWadjetEyeSteam::GetAverageRateStat);
	SCRIPT_METHOD(Steam::SetIntStat^2, AGSWadjetEyeSteam::SetIntStat);
	SCRIPT_METHOD(Steam::SetFloatStat^2, AGSWadjetEyeSteam::SetFloatStat);
	SCRIPT_METHOD(Steam::UpdateAverageRateStat^3, AGSWadjetEyeSteam::UpdateAverageRateStat);
	SCRIPT_METHOD(Steam::ResetStatsAndAchievements^0, AGSWadjetEyeSteam::ResetStatsAndAchievements);
	SCRIPT_METHOD(Steam::get_Initialized, AGSWadjetEyeSteam::get_Initialized);
	SCRIPT_METHOD(Steam::get_CurrentLeaderboardName, AGSWadjetEyeSteam::get_CurrentLeaderboardName);
	SCRIPT_METHOD(Steam::RequestLeaderboard^3, AGSWadjetEyeSteam::RequestLeaderboard);
	SCRIPT_METHOD(Steam::UploadScore^1, AGSWadjetEyeSteam::UploadScore);
	SCRIPT_METHOD(Steam::geti_LeaderboardNames, AGSWadjetEyeSteam::geti_LeaderboardNames);
	SCRIPT_METHOD(Steam::geti_LeaderboardScores, AGSWadjetEyeSteam::geti_LeaderboardScores);
	SCRIPT_METHOD(Steam::get_LeaderboardCount, AGSWadjetEyeSteam::get_LeaderboardCount);
	SCRIPT_METHOD(Steam::GetUserName^0, AGSWadjetEyeSteam::GetUserName);
	SCRIPT_METHOD(Steam::GetCurrentGameLanguage^0, AGSWadjetEyeSteam::GetCurrentGameLanguage);
	SCRIPT_METHOD(Steam::FindLeaderboard^1, AGSWadjetEyeSteam::FindLeaderboard);
	SCRIPT_METHOD(Steam::Initialize^2, AGSWadjetEyeSteam::Initialize);
}

} // namespace AGSGalaxySteam
} // namespace Plugins
} // namespace AGS3
