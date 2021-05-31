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

#ifndef TWINE_ACHIEVEMENTS_H
#define TWINE_ACHIEVEMENTS_H

#include "common/achievements.h"

namespace TwinE {

struct AchievementDescriptionList {
	const char *gameId;
	Common::AchievementsPlatform platform;
	const char *appId;
	const Common::StatDescription stats[64];
	const Common::AchievementDescription descriptions[64];
};

#define ACHIEVEMENT_SIMPLE_ENTRY(id, title, comment) \
	{ id, false, title, comment }
#define ACHIEVEMENT_NODESC_ENTRY(id, title) \
	{ id, false, title, "" }
#define ACHIEVEMENT_HIDDEN_ENTRY(id, title) \
	{ id, true, title, "" }
#define ACHIEVEMENTS_LISTEND \
	{ 0, 0, 0, 0 }

#define STATS_SIMPLE_ENTRY(id, comment, default) \
	{id, comment, default}
#define STATS_NODESC_ENTRY(id, default) \
	{id, "", default}
#define STATS_LISTEND \
	{0, 0, 0}
#define NOSTATS \
	{STATS_LISTEND}

static const AchievementDescriptionList achievementDescriptionList[] = {
	{
		// https://steamdb.info/app/397330/stats/
		"lba",
		Common::STEAM_ACHIEVEMENTS,
		"397330",
		NOSTATS,
		{
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_001", "Victory!", "Finish the game."),
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_002", "Heavily armed", "Get the magic saber."),
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_003", "Collector", "Unlock all the clover boxes."),
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_004", "FIRE !", "Fire the canon in the museum."),
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_005", "New record", "Finish the game in less than 4h."),
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_006", "Double vision", "Get into the meca-pinguin room."),
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_007", "I do what I want!", "Get the syrup in the pharmacy before going to Twinsen's house."),
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_008", "Fanboy", "Talk to the Star Wars fanboy."),
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_009", "Secret breaker", "Find the secret passage in the museum."),
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_010", "Seasickness", "Unlock the second video of the ferry trip."),
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_011", "Croesus", "Have at least 500 Kashes in your wallet."),
			ACHIEVEMENT_SIMPLE_ENTRY("LBA_ACH_012", "In your face", "Unlock the 4 videos in which Twinsen gets a slap."),
			ACHIEVEMENTS_LISTEND
		}
	},

	{0, Common::UNK_ACHIEVEMENTS, 0, {STATS_LISTEND}, {ACHIEVEMENTS_LISTEND}}};

} // namespace TwinE

#undef ACHIEVEMENT_SIMPLE_ENTRY
#undef ACHIEVEMENT_NODESC_ENTRY
#undef ACHIEVEMENT_HIDDEN_ENTRY
#undef ACHIEVEMENTS_LISTEND

#undef STATS_SIMPLE_ENTRY
#undef STATS_NODESC_ENTRY
#undef STATS_LISTEND
#undef NOSTATS

#endif // TWINE_ACHIEVEMENTS_H
