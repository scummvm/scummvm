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

#ifndef COMMON_ACHIEVEMENTS_H
#define COMMON_ACHIEVEMENTS_H

#include "common/array.h"
#include "common/ini-file.h"
#include "common/singleton.h"
#include "common/str.h"

namespace Common {

/**
 * List of game achievements provider platforms.
 * Possible candidates are XBOX Gamerscore, PSN Trophies, Kongregate Badges, etc...
 */
enum AchievementsPlatform {
	STEAM_ACHIEVEMENTS,
	GALAXY_ACHIEVEMENTS,
	UNK_ACHIEVEMENTS = -1
};


/**
 * Per-game achievements information structure item.
 */
struct AchievementDescription {
	const char *id;            // achievement internal id, e.g. "ACHIEVEMENT_TIMING"
	bool isHidden;             // achievement is hidden
	const char *title;         // achievement displayed text, e.g. "Marathon Runner"
	const char *comment;       // optional achievement hint / comment, e.g. "Finish the game in less than 4 hours"
};


/**
 * Per-game achievements information structure item.
 */
struct AchievementsInfo {
	Common::AchievementsPlatform platform;              // achievements platform, e.g. STEAM_ACHIEVEMENTS
	Common::String appId;                               // achievements application ID of given platform
	Common::Array<AchievementDescription> descriptions; // descriptions of all game achievements

	AchievementsInfo() {platform = Common::UNK_ACHIEVEMENTS;}
};


class AchievementsManager : public Singleton<AchievementsManager> {
public:
	AchievementsManager();
	~AchievementsManager();

	bool setActiveDomain(AchievementsPlatform platform, const String &appId);
	bool unsetActiveDomain();
	bool isReady() { return _iniFile != nullptr; }

	// Methods to manipulate individual achievements
	bool setAchievement(const String &id, const String &displayedMessage);
	bool isAchieved(const String &id);
	bool clearAchievement(const String &id);

	// Methods to manipulate individual statistics
	int getStatInt(const String &id);
	bool setStatInt(const String &id, int value);
	float getStatFloat(const String &id);
	bool setStatFloat(const String &id, float value);

	// Methods to reset everything
	bool resetAllAchievements();
	bool resetAllStats();

private:
	INIFile *_iniFile;
	String _iniFileName;
};

/** Shortcut for accessing the achievements manager. */
#define AchMan Common::AchievementsManager::instance()


} // End of namespace Common

#endif // #ifndef COMMON_ACHIEVEMENTS_MANAGER_H
