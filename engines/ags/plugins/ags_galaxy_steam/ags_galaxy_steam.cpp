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
}

NumberPtr AGS2Client::IsAchievementAchieved(const ScriptMethodParams &params) {
	return false;
}

NumberPtr AGS2Client::SetAchievementAchieved(const ScriptMethodParams &params) {
	return false;
}

NumberPtr AGS2Client::ResetAchievement(const ScriptMethodParams &params) {
	return false;
}

NumberPtr AGS2Client::GetIntStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGS2Client::GetFloatStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGS2Client::GetAverageRateStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGS2Client::SetIntStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGS2Client::SetFloatStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGS2Client::UpdateAverageRateStat(const ScriptMethodParams &params) {
	return 0;
}

void AGS2Client::ResetStatsAndAchievements(const ScriptMethodParams &params) {
}

NumberPtr AGS2Client::get_Initialized(const ScriptMethodParams &params) {
	return 0;
}

void AGS2Client::get_CurrentLeaderboardName(const ScriptMethodParams &params) {
}

void AGS2Client::RequestLeaderboard(const ScriptMethodParams &params) {
}

NumberPtr AGS2Client::UploadScore(const ScriptMethodParams &params) {
	return 0;
}

void AGS2Client::geti_LeaderboardNames(const ScriptMethodParams &params) {
}

NumberPtr AGS2Client::geti_LeaderboardScores(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGS2Client::get_LeaderboardCount(const ScriptMethodParams &params) {
	return 0;
}

void AGS2Client::GetUserName(const ScriptMethodParams &params) {
}

void AGS2Client::GetCurrentGameLanguage(const ScriptMethodParams &params) {
}

void AGS2Client::FindLeaderboard(const ScriptMethodParams &params) {
}

NumberPtr AGS2Client::Initialize(const ScriptMethodParams &params) {
	return 0;
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

	const MetaEngine &meta = ::AGS::g_vm->getMetaEngine();
	Common::AchievementsInfo achievementsInfo = meta.getAchievementsInfo(::AGS::g_vm->getGameId());
	const Common::String target = achievementsInfo.appId;
	if (!target.empty()) {
		AchMan.setActiveDomain(Common::GALAXY_ACHIEVEMENTS, target);
	} else {
		warning("Unknown game accessing SteamAPI. All achievements will be ignored.");
		AchMan.unsetActiveDomain();
	}
}

NumberPtr AGSGalaxy::IsAchievementAchieved(const ScriptMethodParams &params) {
	PARAMS1(char *, id);
	return AchMan.isAchieved(id);
}

NumberPtr AGSGalaxy::SetAchievementAchieved(const ScriptMethodParams &params) {
	PARAMS1(char *, id);

	const MetaEngine &meta = ::AGS::g_vm->getMetaEngine();
	Common::AchievementsInfo achievementsInfo = meta.getAchievementsInfo(::AGS::g_vm->getGameId());

	Common::String msg = id;
	for (uint32 i = 0; i < achievementsInfo.descriptions.size(); i++) {
		if (strcmp(achievementsInfo.descriptions[i].id, id) == 0) {
			msg = achievementsInfo.descriptions[i].title;
		}
	}

	return AchMan.setAchievement(id, msg);
}

NumberPtr AGSGalaxy::ResetAchievement(const ScriptMethodParams &params) {
	PARAMS1(char *, id);
	return AchMan.clearAchievement(id);
}

NumberPtr AGSGalaxy::GetIntStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSGalaxy::GetFloatStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSGalaxy::GetAverageRateStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSGalaxy::SetIntStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSGalaxy::SetFloatStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSGalaxy::UpdateAverageRateStat(const ScriptMethodParams &params) {
	return 0;
}

void AGSGalaxy::ResetStatsAndAchievements(const ScriptMethodParams &params) {
	AchMan.resetAllAchievements();
	AchMan.resetAllStats();
}

NumberPtr AGSGalaxy::get_Initialized(const ScriptMethodParams &params) {
	return 0;
}

void AGSGalaxy::get_CurrentLeaderboardName(const ScriptMethodParams &params) {
}

void AGSGalaxy::RequestLeaderboard(const ScriptMethodParams &params) {
}

NumberPtr AGSGalaxy::UploadScore(const ScriptMethodParams &params) {
	return 0;
}

void AGSGalaxy::geti_LeaderboardNames(const ScriptMethodParams &params) {
}

NumberPtr AGSGalaxy::geti_LeaderboardScores(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSGalaxy::get_LeaderboardCount(const ScriptMethodParams &params) {
	return 0;
}

void AGSGalaxy::GetUserName(const ScriptMethodParams &params) {
}

void AGSGalaxy::GetCurrentGameLanguage(const ScriptMethodParams &params) {
}

NumberPtr AGSGalaxy::Initialize(const ScriptMethodParams &params) {
	return 0;
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

	const MetaEngine &meta = ::AGS::g_vm->getMetaEngine();
	Common::AchievementsInfo achievementsInfo = meta.getAchievementsInfo(::AGS::g_vm->getGameId());
	const Common::String target = achievementsInfo.appId;
	if (!target.empty()) {
		AchMan.setActiveDomain(Common::STEAM_ACHIEVEMENTS, target);
	} else {
		warning("Unknown game accessing SteamAPI. All achievements will be ignored.");
		AchMan.unsetActiveDomain();
	}
}

NumberPtr AGSSteam::IsAchievementAchieved(const ScriptMethodParams &params) {
	PARAMS1(char *, id);
	return AchMan.isAchieved(id);
}

NumberPtr AGSSteam::SetAchievementAchieved(const ScriptMethodParams &params) {
	PARAMS1(char *, id);

	const MetaEngine &meta = ::AGS::g_vm->getMetaEngine();
	Common::AchievementsInfo achievementsInfo = meta.getAchievementsInfo(::AGS::g_vm->getGameId());

	Common::String msg = id;
	for (uint32 i = 0; i < achievementsInfo.descriptions.size(); i++) {
		if (strcmp(achievementsInfo.descriptions[i].id, id) == 0) {
			msg = achievementsInfo.descriptions[i].title;
		}
	}

	return AchMan.setAchievement(id, msg);
}

NumberPtr AGSSteam::ResetAchievement(const ScriptMethodParams &params) {
	PARAMS1(char *, id);
	return AchMan.clearAchievement(id);
}

NumberPtr AGSSteam::GetIntStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSSteam::GetFloatStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSSteam::GetAverageRateStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSSteam::SetIntStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSSteam::SetFloatStat(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSSteam::UpdateAverageRateStat(const ScriptMethodParams &params) {
	return 0;
}

void AGSSteam::ResetStatsAndAchievements(const ScriptMethodParams &params) {
	AchMan.resetAllAchievements();
	AchMan.resetAllStats();
}

NumberPtr AGSSteam::get_Initialized(const ScriptMethodParams &params) {
	return 0;
}

void AGSSteam::get_CurrentLeaderboardName(const ScriptMethodParams &params) {
}

void AGSSteam::RequestLeaderboard(const ScriptMethodParams &params) {
}

NumberPtr AGSSteam::UploadScore(const ScriptMethodParams &params) {
	return 0;
}

void AGSSteam::geti_LeaderboardNames(const ScriptMethodParams &params) {
}

NumberPtr AGSSteam::geti_LeaderboardScores(const ScriptMethodParams &params) {
	return 0;
}

NumberPtr AGSSteam::get_LeaderboardCount(const ScriptMethodParams &params) {
	return 0;
}

void AGSSteam::GetUserName(const ScriptMethodParams &params) {
}

void AGSSteam::GetCurrentGameLanguage(const ScriptMethodParams &params) {
}

NumberPtr AGSSteam::FindLeaderboard(const ScriptMethodParams &params) {
	return 0;
}

} // namespace AGSGalaxySteam
} // namespace Plugins
} // namespace AGS3
