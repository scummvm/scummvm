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

#include "ags/plugins/ags_galaxy_steam/ags_wadjeteye_steam.h"

namespace AGS3 {
namespace Plugins {
namespace AGSGalaxySteam {

AGSWadjetEyeSteam::AGSWadjetEyeSteam() : AGSSteam() {
	DLL_METHOD(AGS_EngineStartup);
}

void AGSWadjetEyeSteam::AddAchievement(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSWadjetEyeSteam::AddStat(ScriptMethodParams &params) {
	params._result = 0;
}

void AGSWadjetEyeSteam::AGS_EngineStartup(IAGSEngine *engine) {
	AGSSteam::AGS_EngineStartup(engine);
	SCRIPT_METHOD_EXT(Steam::AddAchievement^1, AddAchievement);
	SCRIPT_METHOD_EXT(Steam::AddStat^2, AddStat);
	SCRIPT_METHOD_EXT(Steam::IsAchievementAchieved^1, IsAchievementAchieved);
	SCRIPT_METHOD_EXT(Steam::SetAchievementAchieved^1, SetAchievementAchieved);
	SCRIPT_METHOD_EXT(Steam::ResetAchievement^1, ResetAchievement);
	SCRIPT_METHOD_EXT(Steam::GetIntStat^1, GetIntStat);
	SCRIPT_METHOD_EXT(Steam::GetFloatStat^1, GetFloatStat);
	SCRIPT_METHOD_EXT(Steam::GetAverageRateStat^1, GetAverageRateStat);
	SCRIPT_METHOD_EXT(Steam::SetIntStat^2, SetIntStat);
	SCRIPT_METHOD_EXT(Steam::SetFloatStat^2, SetFloatStat);
	SCRIPT_METHOD_EXT(Steam::UpdateAverageRateStat^3, UpdateAverageRateStat);
	SCRIPT_METHOD_EXT(Steam::ResetStatsAndAchievements^0, ResetStatsAndAchievements);
	SCRIPT_METHOD_EXT(Steam::get_Initialized, get_Initialized);
	SCRIPT_METHOD_EXT(Steam::get_CurrentLeaderboardName, get_CurrentLeaderboardName);
	SCRIPT_METHOD_EXT(Steam::RequestLeaderboard^3, RequestLeaderboard);
	SCRIPT_METHOD_EXT(Steam::UploadScore^1, UploadScore);
	SCRIPT_METHOD_EXT(Steam::geti_LeaderboardNames, geti_LeaderboardNames);
	SCRIPT_METHOD_EXT(Steam::geti_LeaderboardScores, geti_LeaderboardScores);
	SCRIPT_METHOD_EXT(Steam::get_LeaderboardCount, get_LeaderboardCount);
	SCRIPT_METHOD_EXT(Steam::GetUserName^0, GetUserName);
	SCRIPT_METHOD_EXT(Steam::GetCurrentGameLanguage^0, GetCurrentGameLanguage);
	SCRIPT_METHOD_EXT(Steam::FindLeaderboard^1, FindLeaderboard);
	SCRIPT_METHOD_EXT(Steam::Initialize^2, Initialize);
}

} // namespace AGSGalaxySteam
} // namespace Plugins
} // namespace AGS3
