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
 * @defgroup common_achieve Achievements
 * @ingroup common
 *
 * @brief API related to in-game achievements.
 *
 * @{
 */

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
 * Information structure for game-specific achievements.
 */
struct AchievementDescription {
	const char *id;            //!< Achievement internal ID, such as "ACHIEVEMENT_TIMING".
	bool isHidden;             //!< Whether the achievement is hidden.
	const char *title;         //!< Achievement displayed text, such as "Marathon Runner".
	const char *comment;       //!< Optional achievement hint or comment, such as "Finish the game in less than 4 hours".
};


/**
 * Information structure for platform-specific achievements.
 */
struct AchievementsInfo {
	Common::AchievementsPlatform platform;              //!< Achievements platform, such as "STEAM_ACHIEVEMENTS".
	Common::String appId;                               //!< Achievements application ID of the given platform.
	Common::Array<AchievementDescription> descriptions; //!< Descriptions of all game achievements.

	AchievementsInfo() {platform = Common::UNK_ACHIEVEMENTS;}
};

/**
 * Class for manipulating the achievements.
 *
 * Use the Achievements Manager class to edit the in-game achievements.
 */

class AchievementsManager : public Singleton<AchievementsManager> {
public:
	AchievementsManager();
	~AchievementsManager();
    
    /**
     * Set a platform and application ID as active domain.
     *
     * @param[in] platform Achievements platform.
     * @param[in] appId    Achievements application ID of the given platform.
     */
	bool setActiveDomain(AchievementsPlatform platform, const String &appId);
	bool unsetActiveDomain();                      //!< Unset the current active domain.
	bool isReady() { return _iniFile != nullptr; } //!< Check whether the domain is ready.

	/**
     * @name Methods for manipulating individual achievements
     * @{
     */
	 
	/** Set an achievement.
     *
     * @param[in] id               Internal ID of the achievement.
     * @param[in] displayedMessage Message displayed when the achievement is achieved.
     */

	bool setAchievement(const String &id, const String &displayedMessage);
	
    /**
     * Set an achievement as achieved.
     *
     * @param[in] id Internal ID of the achievement.
     */	
	
	bool isAchieved(const String &id);
	
    /**
     * Clear an achieved achievement.
     *
     * @param[in] id Internal ID of the achievement.
     */	
	
	bool clearAchievement(const String &id);
    
    /** @} */
	
    /**
     * @name Methods for manipulating individual statistics
     * @{
     */

    /**
     * Get a statistic (integer).
     *
     * @param[in] id Internal ID of the achievement.
     */	
	 
	int getStatInt(const String &id);
	
    /**
     * Set a statistic to an integer number.
     *
     * @param[in] id    Internal ID of the achievement.
     * @param[in] value Value to which the statistic is set.
     */	
	
	bool setStatInt(const String &id, int value);
	
    /**
     * Get a statistic (float).
     *
     * @param[in] id    Internal ID of the achievement.
     */	
	
	float getStatFloat(const String &id);
	
    /**
     * Set a statistic to a float number.
     *
     * @param[in] id    Internal ID of the achievement.
     * @param[in] value Value to which the statistic is set.
     */	
	
	bool setStatFloat(const String &id, float value);

    /** @} */

    /**
     * @name Methods for resetting achievements and statistics
     * @{
     */
	bool resetAllAchievements(); //!< Reset all achievements.
	bool resetAllStats();        //!< Reset all statistics.

    /** @} */

private:
	INIFile *_iniFile;
	String _iniFileName;
};

/** Shortcut for accessing the Achievements Manager. */
#define AchMan Common::AchievementsManager::instance()

/** @} */

} // End of namespace Common

#endif // #ifndef COMMON_ACHIEVEMENTS_MANAGER_H
