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

#ifndef COMMON_ACHIEVEMENTS_H
#define COMMON_ACHIEVEMENTS_H

#include "common/array.h"
#include "common/formats/ini-file.h"
#include "common/hashmap.h"
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
 * Information structure for game-specific statistics.
 */
struct StatDescription {
	String id;      //!< Stat internal ID, such as "ITEMS_THROWN".
	String comment; //!< Optional stat comment, such as "Items Thrown".
	String start;   //!< Stat default value, such as "0".
};

/**
 * Information structure for game-specific achievements.
 */
struct AchievementDescription {
	String id;      //!< Achievement internal ID, such as "ACHIEVEMENT_TIMING".
	String title;   //!< Achievement displayed text, such as "Marathon Runner".
	String comment; //!< Optional achievement hint or comment, such as "Finish the game in less than 4 hours".
	bool isHidden;  //!< Whether the achievement is hidden.
};

/**
 * Information structure for platform-specific achievements.
 */
struct AchievementsInfo {
	Common::AchievementsPlatform platform;              //!< Achievements platform, such as "STEAM_ACHIEVEMENTS".
	Common::String appId;                               //!< Achievements application ID of the given platform.

	AchievementsInfo() { platform = Common::UNK_ACHIEVEMENTS; }
};

/**
 * The meta engine returns an array of achievement descriptions
 *
 * @note This array (if not @c nullptr) must end on a terminating entry
 * @sa ACHIEVEMENT_DESC_TABLE_END_MARKER
 */
struct AchievementDescriptionList {
	const char *gameId;
	Common::AchievementsPlatform platform;
	const char *appId;
};

#define ACHIEVEMENT_DESC_TABLE_END_MARKER \
	{ nullptr, Common::UNK_ACHIEVEMENTS, nullptr }

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
	 * Set a game targeted by platform type and application ID as active domain.
	 * Automaticly loads messages texts from achievements.dat.
	 *
	 * @param[in] info Achievements platform type and application ID.
	 */
	bool setActiveDomain(const AchievementsInfo &info);
	bool unsetActiveDomain();                            //!< Unset the current active domain.
	bool isReady() const { return _iniFile != nullptr; } //!< Check whether the domain is ready.

	/**
	 * @name Methods for manipulating individual achievements
	 * @{
	 */

	/** Set an achievement. Message is automatically displayed with text from active domain.
	 *
	 * @param[in] id			   Internal ID of the achievement.
	 */
	bool setAchievement(const String &id);

	/**
	 * Check if an achievement as achieved.
	 *
	 * @param[in] id Internal ID of the achievement.
	 */
	bool isAchieved(const String &id) const;

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
	int getStatInt(const String &id) const;

	/**
	 * Set a statistic to an integer number.
	 *
	 * @param[in] id	Internal ID of the achievement.
	 * @param[in] value Value to which the statistic is set.
	 */
	bool setStatInt(const String &id, int value);

	/**
	 * Get a statistic (float).
	 *
	 * @param[in] id	Internal ID of the achievement.
	 */
	float getStatFloat(const String &id) const;

	/**
	 * Set a statistic to a float number.
	 *
	 * @param[in] id	Internal ID of the achievement.
	 * @param[in] value Value to which the statistic is set.
	 */
	bool setStatFloat(const String &id, float value);

	/**
	 * Get a statistic (raw string).
	 *
	 * @param[in] id	Internal ID of the achievement.
	 */
	const String getStatRaw(const String &id) const;

	/**
	 * Get an average rate statistic (float).
	 * Calcucated by devision the sum of count by the sum of times.
	 *
	 * @param[in] id	Internal ID of the achievement.
	 */
	float getAverageRateStatFloat(const String &id) const;

	/**
	 * Update an average rate statistic (float).
	 *
	 * @param[in] id	Internal ID of the achievement.
	 * @param[in] count Value to which the statistic count is increased.
	 * @param[in] times Value to which the statistic times is increased.
	 */
	bool updateAverageRateStatFloat(const String &id, float count, float times);

	/** @} */

	/**
	 * @name Methods for resetting achievements and statistics
	 * @{
	 */
	bool resetAllAchievements(); //!< Reset all achievements.
	bool resetAllStats();        //!< Reset all statistics.

	/** @} */

	/**
	 * @name Methods for storing platform-specific data
	 * @{
	 */

	/**
	 * Store provided key and value pair in additional section.
	 * May be useful for posting achievements to original platform.
	 *
	 * @param[in] id	Internal ID of the achievement.
	 * @param[in] value Value to which the statistic is set.
	 */
	bool setSpecialString(const String &id, const String &value);

	/** @} */

	/**
	 * @name Methods for getting achievements and statistics descriptions
	 * @{
	 */

	/**
	 * Get number of achievement descriptions available.
	 *
	 */
	uint16 getAchievementCount() const;

	/**
	 * Get achievement description by index.
	 *
	 * @param[in] index	Internal index of the achievement, counted from 0 to (getAchievementCount() - 1)
	 *
	 */
	const AchievementDescription *getAchievementDescription(uint16 index) const;

	/**
	 * Get number of stat descriptions available.
	 *
	 */
	uint16 getStatCount() const;

	/**
	 * Get stat description by index.
	 *
	 * @param[in] index	Internal index of the stat, counted from 0 to (getStatCount() - 1)
	 *
	 */
	const StatDescription *getStatDescription(uint16 index) const;

	/** @} */

private:
	String getCurrentLang() const;
	bool loadAchievementsData(const char *platform, const char *appId);

	float getStatFloatEx(const String &id, const String &section) const;
	bool setStatFloatEx(const String &id, float value, const String &section) const;

	INIFile *_iniFile;
	String _iniFileName;
	Common::Array<StatDescription> _stats;
	Common::HashMap<String, Common::Array<AchievementDescription> > _achievements;
};

/** Shortcut for accessing the Achievements Manager. */
#define AchMan Common::AchievementsManager::instance()

/** @} */

} // End of namespace Common

#endif // #ifndef COMMON_ACHIEVEMENTS_MANAGER_H
