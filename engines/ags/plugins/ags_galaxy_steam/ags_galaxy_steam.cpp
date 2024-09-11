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

#include "ags/metaengine.h"
#include "ags/ags.h"
#include "ags/plugins/ags_galaxy_steam/ags_galaxy_steam.h"
#include "common/config-manager.h"

namespace AGS3 {
namespace Plugins {
namespace AGSGalaxySteam {

struct SteamData : public IAGSScriptManagedObject {
public:
	Common::String steamLanguage = "english";

	int Dispose(void *address, bool force) override {
		delete this;
		return true;
	}

	const char *GetType() override {
		return "SteamData";
	};

	int Serialize(void *address, char *buffer, int bufsize) override {
		return 0;
	}
};

void AGS2Client::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(AGS2Client::IsAchievementAchieved^1, AGS2Client::IsAchievementAchieved);
	SCRIPT_METHOD(AGS2Client::SetAchievementAchieved^1, AGS2Client::SetAchievementAchieved);
	SCRIPT_METHOD(AGS2Client::ResetAchievement^1, AGS2Client::ResetAchievement);
	SCRIPT_METHOD(AGS2Client::GetIntStat^1, AGS2Client::GetIntStat);
	SCRIPT_METHOD(AGS2Client::GetFloatStat^1, AGS2Client::GetFloatStat);
	SCRIPT_METHOD(AGS2Client::GetAverageRateStat^1, AGS2Client::GetAverageRateStat);
	SCRIPT_METHOD(AGS2Client::SetIntStat^2, AGS2Client::SetIntStat);
	SCRIPT_METHOD(AGS2Client::SetFloatStat^2, AGS2Client::SetFloatStat);
	SCRIPT_METHOD(AGS2Client::UpdateAverageRateStat^3, AGS2Client::UpdateAverageRateStat);
	SCRIPT_METHOD(AGS2Client::ResetStatsAndAchievements^0, AGS2Client::ResetStatsAndAchievements);
	SCRIPT_METHOD(AGS2Client::get_Initialized, AGS2Client::get_Initialized);
	SCRIPT_METHOD(AGS2Client::get_CurrentLeaderboardName, AGS2Client::get_CurrentLeaderboardName);
	SCRIPT_METHOD(AGS2Client::RequestLeaderboard^3, AGS2Client::RequestLeaderboard);
	SCRIPT_METHOD(AGS2Client::UploadScore^1, AGS2Client::UploadScore);
	SCRIPT_METHOD(AGS2Client::geti_LeaderboardNames, AGS2Client::geti_LeaderboardNames);
	SCRIPT_METHOD(AGS2Client::geti_LeaderboardScores, AGS2Client::geti_LeaderboardScores);
	SCRIPT_METHOD(AGS2Client::get_LeaderboardCount, AGS2Client::get_LeaderboardCount);
	SCRIPT_METHOD(AGS2Client::GetUserName^0, AGS2Client::GetUserName);
	SCRIPT_METHOD(AGS2Client::GetCurrentGameLanguage^0, AGS2Client::GetCurrentGameLanguage);
	SCRIPT_METHOD(AGS2Client::FindLeaderboard^1, AGS2Client::FindLeaderboard);
	SCRIPT_METHOD(AGS2Client::Initialize^2, AGS2Client::Initialize);

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
	warning("AGS2Client::GetUserName() is not implemented - Returning \'Player\'");
	params._result = _engine->CreateScriptString("Player");
}

void AGS2Client::GetCurrentGameLanguage(ScriptMethodParams &params) {
	SteamData *steam_data = new SteamData();
	_engine->RegisterManagedObject(steam_data, steam_data);
	warning("AGS2Client::GetCurrentGameLanguage() is not implemented - Returning \'%s\'", steam_data->steamLanguage.c_str());
	params._result = steam_data->steamLanguage.c_str();
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

const char *AGSGalaxy::AGS_GetPluginName() {
	return "AGSGalaxy";
}

void AGSGalaxy::AGS_EngineStartup(IAGSEngine *engine) {
	AGS2Client::AGS_EngineStartup(engine);

	SCRIPT_METHOD(AGSGalaxy::IsAchievementAchieved^1, AGSGalaxy::IsAchievementAchieved);
	SCRIPT_METHOD(AGSGalaxy::SetAchievementAchieved^1, AGSGalaxy::SetAchievementAchieved);
	SCRIPT_METHOD(AGSGalaxy::ResetAchievement^1, AGSGalaxy::ResetAchievement);
	SCRIPT_METHOD(AGSGalaxy::GetIntStat^1, AGSGalaxy::GetIntStat);
	SCRIPT_METHOD(AGSGalaxy::GetFloatStat^1, AGSGalaxy::GetFloatStat);
	SCRIPT_METHOD(AGSGalaxy::GetAverageRateStat^1, AGSGalaxy::GetAverageRateStat);
	SCRIPT_METHOD(AGSGalaxy::SetIntStat^2, AGSGalaxy::SetIntStat);
	SCRIPT_METHOD(AGSGalaxy::SetFloatStat^2, AGSGalaxy::SetFloatStat);
	SCRIPT_METHOD(AGSGalaxy::UpdateAverageRateStat^3, AGSGalaxy::UpdateAverageRateStat);
	SCRIPT_METHOD(AGSGalaxy::ResetStatsAndAchievements^0, AGSGalaxy::ResetStatsAndAchievements);
	SCRIPT_METHOD(AGSGalaxy::get_Initialized, AGSGalaxy::get_Initialized);
	SCRIPT_METHOD(AGSGalaxy::get_CurrentLeaderboardName, AGSGalaxy::get_CurrentLeaderboardName);
	SCRIPT_METHOD(AGSGalaxy::RequestLeaderboard^3, AGSGalaxy::RequestLeaderboard);
	SCRIPT_METHOD(AGSGalaxy::UploadScore^1, AGSGalaxy::UploadScore);
	SCRIPT_METHOD(AGSGalaxy::geti_LeaderboardNames, AGSGalaxy::geti_LeaderboardNames);
	SCRIPT_METHOD(AGSGalaxy::geti_LeaderboardScores, AGSGalaxy::geti_LeaderboardScores);
	SCRIPT_METHOD(AGSGalaxy::get_LeaderboardCount, AGSGalaxy::get_LeaderboardCount);
	SCRIPT_METHOD(AGSGalaxy::GetUserName^0, AGSGalaxy::GetUserName);
	SCRIPT_METHOD(AGSGalaxy::GetCurrentGameLanguage^0, AGSGalaxy::GetCurrentGameLanguage);
	SCRIPT_METHOD(AGSGalaxy::Initialize^2, AGSGalaxy::Initialize);
}

/*------------------------------------------------------------------*/

const char *AGSSteam::AGS_GetPluginName() {
	return "AGSSteam";
}

void AGSSteam::AGS_EngineStartup(IAGSEngine *engine) {
	AGS2Client::AGS_EngineStartup(engine);

	SCRIPT_METHOD(AGSteam::IsAchievementAchieved^1, AGSSteam::IsAchievementAchieved);
	SCRIPT_METHOD(AGSteam::SetAchievementAchieved^1, AGSSteam::SetAchievementAchieved);
	SCRIPT_METHOD(AGSteam::ResetAchievement^1, AGSSteam::ResetAchievement);
	SCRIPT_METHOD(AGSteam::GetIntStat^1, AGSSteam::GetIntStat);
	SCRIPT_METHOD(AGSteam::GetFloatStat^1, AGSSteam::GetFloatStat);
	SCRIPT_METHOD(AGSteam::GetAverageRateStat^1, AGSSteam::GetAverageRateStat);
	SCRIPT_METHOD(AGSteam::SetIntStat^2, AGSSteam::SetIntStat);
	SCRIPT_METHOD(AGSteam::SetFloatStat^2, AGSSteam::SetFloatStat);
	SCRIPT_METHOD(AGSteam::UpdateAverageRateStat^3, AGSSteam::UpdateAverageRateStat);
	SCRIPT_METHOD(AGSteam::ResetStatsAndAchievements^0, AGSSteam::ResetStatsAndAchievements);
	SCRIPT_METHOD(AGSteam::get_Initialized, AGSSteam::get_Initialized);
	SCRIPT_METHOD(AGSteam::get_CurrentLeaderboardName, AGSSteam::get_CurrentLeaderboardName);
	SCRIPT_METHOD(AGSteam::RequestLeaderboard^3, AGSSteam::RequestLeaderboard);
	SCRIPT_METHOD(AGSteam::UploadScore^1, AGSSteam::UploadScore);
	SCRIPT_METHOD(AGSteam::geti_LeaderboardNames, AGSSteam::geti_LeaderboardNames);
	SCRIPT_METHOD(AGSteam::geti_LeaderboardScores, AGSSteam::geti_LeaderboardScores);
	SCRIPT_METHOD(AGSteam::get_LeaderboardCount, AGSSteam::get_LeaderboardCount);
	SCRIPT_METHOD(AGSteam::GetUserName^0, AGSSteam::GetUserName);
	SCRIPT_METHOD(AGSteam::GetCurrentGameLanguage^0, AGSSteam::GetCurrentGameLanguage);
	SCRIPT_METHOD(AGSteam::FindLeaderboard^1, AGSSteam::FindLeaderboard);
}

} // namespace AGSGalaxySteam
} // namespace Plugins
} // namespace AGS3
