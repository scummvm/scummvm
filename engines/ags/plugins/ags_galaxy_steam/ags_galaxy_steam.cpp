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

#include "ags/metaengine.h"
#include "ags/ags.h"
#include "ags/plugins/ags_galaxy_steam/ags_galaxy_steam.h"
#include "common/config-manager.h"

namespace AGS3 {
namespace Plugins {
namespace AGSGalaxySteam {

AGS2Client::AGS2Client() : PluginBase() {
	DLL_METHOD(AGS_EngineStartup);
}

void AGS2Client::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(AGS2Client::IsAchievementAchieved^1, IsAchievementAchieved);
	SCRIPT_METHOD_EXT(AGS2Client::SetAchievementAchieved^1, SetAchievementAchieved);
	SCRIPT_METHOD_EXT(AGS2Client::ResetAchievement^1, ResetAchievement);
	SCRIPT_METHOD_EXT(AGS2Client::GetIntStat^1, GetIntStat);
	SCRIPT_METHOD_EXT(AGS2Client::GetFloatStat^1, GetFloatStat);
	SCRIPT_METHOD_EXT(AGS2Client::GetAverageRateStat^1, GetAverageRateStat);
	SCRIPT_METHOD_EXT(AGS2Client::SetIntStat^2, SetIntStat);
	SCRIPT_METHOD_EXT(AGS2Client::SetFloatStat^2, SetFloatStat);
	SCRIPT_METHOD_EXT(AGS2Client::UpdateAverageRateStat^3, UpdateAverageRateStat);
	SCRIPT_METHOD_EXT(AGS2Client::ResetStatsAndAchievements^0, ResetStatsAndAchievements);
	SCRIPT_METHOD_EXT(AGS2Client::get_Initialized, get_Initialized);
	SCRIPT_METHOD_EXT(AGS2Client::get_CurrentLeaderboardName, get_CurrentLeaderboardName);
	SCRIPT_METHOD_EXT(AGS2Client::RequestLeaderboard^3, RequestLeaderboard);
	SCRIPT_METHOD_EXT(AGS2Client::UploadScore^1, UploadScore);
	SCRIPT_METHOD_EXT(AGS2Client::geti_LeaderboardNames, geti_LeaderboardNames);
	SCRIPT_METHOD_EXT(AGS2Client::geti_LeaderboardScores, geti_LeaderboardScores);
	SCRIPT_METHOD_EXT(AGS2Client::get_LeaderboardCount, get_LeaderboardCount);
	SCRIPT_METHOD_EXT(AGS2Client::GetUserName^0, GetUserName);
	SCRIPT_METHOD_EXT(AGS2Client::GetCurrentGameLanguage^0, GetCurrentGameLanguage);
	SCRIPT_METHOD_EXT(AGS2Client::FindLeaderboard^1, FindLeaderboard);
	SCRIPT_METHOD_EXT(AGS2Client::Initialize^2, Initialize);

	Common::String gameTarget = ConfMan.getActiveDomainName();
	const MetaEngine *meta = ::AGS::g_vm->getMetaEngine();
	AchMan.setActiveDomain(meta->getAchievementsInfo(gameTarget));
}

void AGS2Client::IsAchievementAchieved(ScriptMethodParams &params) {
	PARAMS1(char *, id);
	params._result = AchMan.isAchieved(id);
}

void AGS2Client::SetAchievementAchieved(ScriptMethodParams &params) {
	PARAMS1(char *, id);
	params._result = AchMan.setAchievement(id);
}

void AGS2Client::ResetAchievement(ScriptMethodParams &params) {
	PARAMS1(char *, id);
	params._result = AchMan.clearAchievement(id);
}

void AGS2Client::GetIntStat(ScriptMethodParams &params) {
	PARAMS1(char *, id);
	params._result = AchMan.getStatInt(id);
}

void AGS2Client::GetFloatStat(ScriptMethodParams &params) {
	PARAMS1(char *, id);
	params._result = PARAM_FROM_FLOAT(AchMan.getStatFloat(id));
}

void AGS2Client::GetAverageRateStat(ScriptMethodParams &params) {
	PARAMS1(char *, id);
	params._result = PARAM_FROM_FLOAT(AchMan.getAverageRateStatFloat(id));
}

void AGS2Client::SetIntStat(ScriptMethodParams &params) {
	PARAMS2(char *, id, int, value);
	params._result = AchMan.setStatInt(id, value);
}

void AGS2Client::SetFloatStat(ScriptMethodParams &params) {
	PARAMS2(char *, id, int32, value);
	params._result = AchMan.setStatFloat(id, PARAM_TO_FLOAT(value));
}

void AGS2Client::UpdateAverageRateStat(ScriptMethodParams &params) {
	PARAMS3(char *, id, int32, count, int32, times);
	params._result = AchMan.updateAverageRateStatFloat(id, PARAM_TO_FLOAT(count), PARAM_TO_FLOAT(times));
}

void AGS2Client::ResetStatsAndAchievements(ScriptMethodParams &params) {
	AchMan.resetAllAchievements();
	AchMan.resetAllStats();
}

void AGS2Client::get_Initialized(ScriptMethodParams &params) {
	// TODO: remove this after GetCurrentGameLanguage() is implemented
	if (ConfMan.get("gameid") == "heroinesquest" ||
		ConfMan.get("gameid") == "killyourself"
	) {
		warning("AGS2Client::get_Initialized() is returning fake value to avoid calling GetCurrentGameLanguage() by game");
		params._result = 0;
		return;
	}

	params._result = AchMan.isReady();
}

void AGS2Client::get_CurrentLeaderboardName(ScriptMethodParams &params) {
	warning("AGS2Client::get_CurrentLeaderboardName() is not implemented");
	params._result = 0;
}

void AGS2Client::RequestLeaderboard(ScriptMethodParams &params) {
	warning("AGS2Client::RequestLeaderboard() is not implemented");
	params._result = 0;
}

void AGS2Client::UploadScore(ScriptMethodParams &params) {
	warning("AGS2Client::UploadScore() is not implemented");
	params._result = 0;
}

void AGS2Client::geti_LeaderboardNames(ScriptMethodParams &params) {
	warning("AGS2Client::geti_LeaderboardNames() is not implemented");
	params._result = 0;
}

void AGS2Client::geti_LeaderboardScores(ScriptMethodParams &params) {
	warning("AGS2Client::geti_LeaderboardScores() is not implemented");
	params._result = 0;
}

void AGS2Client::get_LeaderboardCount(ScriptMethodParams &params) {
	warning("AGS2Client::get_LeaderboardCount() is not implemented");
	params._result = 0;
}

void AGS2Client::GetUserName(ScriptMethodParams &params) {
	warning("AGS2Client::GetUserName() is not implemented");
	params._result = 0;
}

void AGS2Client::GetCurrentGameLanguage(ScriptMethodParams &params) {
	warning("AGS2Client::GetCurrentGameLanguage() is not implemented");
	params._result = 0;
}

void AGS2Client::FindLeaderboard(ScriptMethodParams &params) {
	warning("AGS2Client::FindLeaderboard() is not implemented");
	params._result = 0;
}

void AGS2Client::Initialize(ScriptMethodParams &params) {
	PARAMS2(char *, clientId, char *, clientSecret);
	AchMan.setSpecialString("clientId", clientId);
	AchMan.setSpecialString("clientSecret", clientSecret);
	params._result = 0;
}

/*------------------------------------------------------------------*/

AGSGalaxy::AGSGalaxy() : AGS2Client() {
	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSGalaxy::AGS_GetPluginName() {
	return "AGSGalaxy";
}

void AGSGalaxy::AGS_EngineStartup(IAGSEngine *engine) {
	AGS2Client::AGS_EngineStartup(engine);

	SCRIPT_METHOD_EXT(AGSGalaxy::IsAchievementAchieved^1, IsAchievementAchieved);
	SCRIPT_METHOD_EXT(AGSGalaxy::SetAchievementAchieved^1, SetAchievementAchieved);
	SCRIPT_METHOD_EXT(AGSGalaxy::ResetAchievement^1, ResetAchievement);
	SCRIPT_METHOD_EXT(AGSGalaxy::GetIntStat^1, GetIntStat);
	SCRIPT_METHOD_EXT(AGSGalaxy::GetFloatStat^1, GetFloatStat);
	SCRIPT_METHOD_EXT(AGSGalaxy::GetAverageRateStat^1, GetAverageRateStat);
	SCRIPT_METHOD_EXT(AGSGalaxy::SetIntStat^2, SetIntStat);
	SCRIPT_METHOD_EXT(AGSGalaxy::SetFloatStat^2, SetFloatStat);
	SCRIPT_METHOD_EXT(AGSGalaxy::UpdateAverageRateStat^3, UpdateAverageRateStat);
	SCRIPT_METHOD_EXT(AGSGalaxy::ResetStatsAndAchievements^0, ResetStatsAndAchievements);
	SCRIPT_METHOD_EXT(AGSGalaxy::get_Initialized, get_Initialized);
	SCRIPT_METHOD_EXT(AGSGalaxy::get_CurrentLeaderboardName, get_CurrentLeaderboardName);
	SCRIPT_METHOD_EXT(AGSGalaxy::RequestLeaderboard^3, RequestLeaderboard);
	SCRIPT_METHOD_EXT(AGSGalaxy::UploadScore^1, UploadScore);
	SCRIPT_METHOD_EXT(AGSGalaxy::geti_LeaderboardNames, geti_LeaderboardNames);
	SCRIPT_METHOD_EXT(AGSGalaxy::geti_LeaderboardScores, geti_LeaderboardScores);
	SCRIPT_METHOD_EXT(AGSGalaxy::get_LeaderboardCount, get_LeaderboardCount);
	SCRIPT_METHOD_EXT(AGSGalaxy::GetUserName^0, GetUserName);
	SCRIPT_METHOD_EXT(AGSGalaxy::GetCurrentGameLanguage^0, GetCurrentGameLanguage);
	SCRIPT_METHOD_EXT(AGSGalaxy::Initialize^2, Initialize);
}

/*------------------------------------------------------------------*/

AGSSteam::AGSSteam() : AGS2Client() {
	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
}

const char *AGSSteam::AGS_GetPluginName() {
	return "AGSSteam";
}

void AGSSteam::AGS_EngineStartup(IAGSEngine *engine) {
	AGS2Client::AGS_EngineStartup(engine);

	SCRIPT_METHOD_EXT(AGSteam::IsAchievementAchieved^1, IsAchievementAchieved);
	SCRIPT_METHOD_EXT(AGSteam::SetAchievementAchieved^1, SetAchievementAchieved);
	SCRIPT_METHOD_EXT(AGSteam::ResetAchievement^1, ResetAchievement);
	SCRIPT_METHOD_EXT(AGSteam::GetIntStat^1, GetIntStat);
	SCRIPT_METHOD_EXT(AGSteam::GetFloatStat^1, GetFloatStat);
	SCRIPT_METHOD_EXT(AGSteam::GetAverageRateStat^1, GetAverageRateStat);
	SCRIPT_METHOD_EXT(AGSteam::SetIntStat^2, SetIntStat);
	SCRIPT_METHOD_EXT(AGSteam::SetFloatStat^2, SetFloatStat);
	SCRIPT_METHOD_EXT(AGSteam::UpdateAverageRateStat^3, UpdateAverageRateStat);
	SCRIPT_METHOD_EXT(AGSteam::ResetStatsAndAchievements^0, ResetStatsAndAchievements);
	SCRIPT_METHOD_EXT(AGSteam::get_Initialized, get_Initialized);
	SCRIPT_METHOD_EXT(AGSteam::get_CurrentLeaderboardName, get_CurrentLeaderboardName);
	SCRIPT_METHOD_EXT(AGSteam::RequestLeaderboard^3, RequestLeaderboard);
	SCRIPT_METHOD_EXT(AGSteam::UploadScore^1, UploadScore);
	SCRIPT_METHOD_EXT(AGSteam::geti_LeaderboardNames, geti_LeaderboardNames);
	SCRIPT_METHOD_EXT(AGSteam::geti_LeaderboardScores, geti_LeaderboardScores);
	SCRIPT_METHOD_EXT(AGSteam::get_LeaderboardCount, get_LeaderboardCount);
	SCRIPT_METHOD_EXT(AGSteam::GetUserName^0, GetUserName);
	SCRIPT_METHOD_EXT(AGSteam::GetCurrentGameLanguage^0, GetCurrentGameLanguage);
	SCRIPT_METHOD_EXT(AGSteam::FindLeaderboard^1, FindLeaderboard);
}

} // namespace AGSGalaxySteam
} // namespace Plugins
} // namespace AGS3
