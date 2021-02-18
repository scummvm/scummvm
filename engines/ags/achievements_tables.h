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

#ifndef AGS_ACHIEVEMENTS_H
#define AGS_ACHIEVEMENTS_H

namespace AGS {

struct AchievementDescriptionList {
	const char *gameId;
	Common::AchievementsPlatform platform;
	const char *appId;
	const Common::AchievementDescription descriptions[64];
};

#define ACHIEVEMENT_SIMPLE_ENTRY(id, title, comment) {id, false, title, comment}
#define ACHIEVEMENT_NODESC_ENTRY(id, title) {id, false, title, ""}
#define ACHIEVEMENT_HIDDEN_ENTRY(id, title) {id, true, title, ""}
#define ACHIEVEMENTS_LISTEND {0,0,0,0}

static const AchievementDescriptionList achievementDescriptionList[] = {
	{
		"blackwell4",
		Common::STEAM_ACHIEVEMENTS,
		"80360",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("MEDIUM_WELL_DONE", "Medium Well Done", "Complete the game once"),
			ACHIEVEMENT_SIMPLE_ENTRY("TEAMWORK", "Teamwork", "Use both Rosa and Joey to solve a puzzle"),
			ACHIEVEMENT_SIMPLE_ENTRY("GEROMINO", "Geromino!", "Save a ghost... and yourself"),
			ACHIEVEMENT_SIMPLE_ENTRY("KEY_SNATCHER", "Key Snatcher", "Sometimes you've gotta rob a few old ladies..."),
			ACHIEVEMENT_SIMPLE_ENTRY("OUT_OF_SIGHT", "Out of Sight", "Get out of a sticky situation"),
			ACHIEVEMENT_SIMPLE_ENTRY("COMPETITION", "Check out the Competition", "Visit another psychic"),
			ACHIEVEMENT_SIMPLE_ENTRY("DUMPSTER_DIVER", "Dumpster Diver", "Find a lost item"),
			ACHIEVEMENT_SIMPLE_ENTRY("TAKE_MY_CARD", "Take my Card", "Use Rosa's business card for an unintended purpose"),
			ACHIEVEMENT_SIMPLE_ENTRY("MUSIC_LOVER", "Music Lover", "Find Jamie's favorite song"),
			ACHIEVEMENT_SIMPLE_ENTRY("STILL_GOT_IT", "Still Got It", "Not bad, for a dead guy"),
			ACHIEVEMENT_SIMPLE_ENTRY("UNBOUND", "Unbound", "Free Rosa"),
			ACHIEVEMENT_SIMPLE_ENTRY("BACK_FROM_BEYOND", "Back from Beyond", "Come back from the void"),
			ACHIEVEMENT_SIMPLE_ENTRY("FACE_PLANT", "Face Plant", "Abuse an innocent"),
			ACHIEVEMENT_SIMPLE_ENTRY("BUTTON_PUSHER", "Button Pusher", "Open the secret compartment on the first try"),
			ACHIEVEMENT_SIMPLE_ENTRY("LISTEN_CLOSELY", "Listen Closely, Now", "Finish the game in commentary mode"),
			ACHIEVEMENT_SIMPLE_ENTRY("BULLET_DODGER", "Bullet Dodger", "Escape the yacht with only getting shot once"),
			ACHIEVEMENT_SIMPLE_ENTRY("GHOSTS_OF_THE_PAST", "Ghosts of the Past", "Find all the references to previous games in Rosa's apartment"),
			ACHIEVEMENT_SIMPLE_ENTRY("BLOWEYMALLONE", "Blowey Mallone", "Use Joey's breath ability on at least 10 people"),
			ACHIEVEMENTS_LISTEND
		}
	},
	
	{
		"blackwell5",
		Common::STEAM_ACHIEVEMENTS,
		"236930",
		{
			ACHIEVEMENT_SIMPLE_ENTRY("CONVERGENCE", "Convergence", "Two becomes one"),
			ACHIEVEMENT_SIMPLE_ENTRY("CURTAINCALL", "Curtain Call", "Save your first ghost"),
			ACHIEVEMENT_SIMPLE_ENTRY("DECEPTION", "Deception", "Try to lie your way out of it"),
			ACHIEVEMENT_SIMPLE_ENTRY("DUMPSTERJUMPER", "Dumpster Jumper", "Make a leap without falling once"),
			ACHIEVEMENT_SIMPLE_ENTRY("EPIPHANY", "Epiphany", "The hows and whys of Joey"),
			ACHIEVEMENT_SIMPLE_ENTRY("GHOSTLYSILENT", "Ghostly silent", "Keep clam and keep cool"),
			ACHIEVEMENT_SIMPLE_ENTRY("GOLDFARBDIGGER", "Goldfarb Digger", "Find a secret room"),
			ACHIEVEMENT_SIMPLE_ENTRY("JOINEDTHEMUSIC", "Joined the Music", "She has plenty of time..."),
			ACHIEVEMENT_SIMPLE_ENTRY("LAYINGITBEAR", "Laying it Bear", "Learn a long kept secret"),
			ACHIEVEMENT_SIMPLE_ENTRY("LEGACY", "Legacy", "Reunite a family"),
			ACHIEVEMENT_SIMPLE_ENTRY("LISTEN", "Move on and Listen", "Finish the game in commentary mode (from the first screen till the last)"),
			ACHIEVEMENT_SIMPLE_ENTRY("ONLINETROLL", "Online Troll", "Gain access to an online game"),
			ACHIEVEMENT_SIMPLE_ENTRY("SNOWPLOW", "Snow Plow", "Somebody's got to do it..."),
			ACHIEVEMENT_SIMPLE_ENTRY("SOFTLYSOFTLY", "Softly Softly", "Create no more than 450 footprints"),
			ACHIEVEMENT_SIMPLE_ENTRY("TEAMWORK", "Teamwork", "Solve a puzzle together"),
			ACHIEVEMENT_SIMPLE_ENTRY("MOVEON", "Time to Move On", "Complete the story"),
			ACHIEVEMENT_SIMPLE_ENTRY("UNBOUND", "Unbound", "Free yourself"),
			ACHIEVEMENT_SIMPLE_ENTRY("BLOWEYMALLONE", "Blowey Mallone", "Blow on everyone you can find"),
			ACHIEVEMENTS_LISTEND
		}
	},
	
	{0, Common::UNK_ACHIEVEMENTS, 0, {ACHIEVEMENTS_LISTEND}}
};

} // End of namespace AGS

#undef ACHIEVEMENT_SIMPLE_ENTRY
#undef ACHIEVEMENT_NODESC_ENTRY
#undef ACHIEVEMENT_HIDDEN_ENTRY
#undef ACHIEVEMENTS_LISTEND

#endif /* AGS_ACHIEVEMENTS_H */
