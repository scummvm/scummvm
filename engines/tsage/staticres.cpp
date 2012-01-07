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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "tsage/staticres.h"

namespace TsAGE {

const byte CURSOR_ARROW_DATA[] = {
	15, 0, 15, 0, 0, 0, 0, 0, 9, 0,
	0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF,	0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,	0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,	0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09
};

const byte CURSOR_WALK_DATA[] = {
	15, 0, 15, 0, 7, 0, 7, 0, 9, 0,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09,	0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x09,	0x09, 0x09, 0x09, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0x09, 0x09, 0x09, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x09,	0x09, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09,	0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09,	0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09,	0x09,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09
};

char const *const LOOK_SCENE_HOTSPOT = "You see nothing special.";
char const *const USE_SCENE_HOTSPOT = "That accomplishes nothing.";
char const *const TALK_SCENE_HOTSPOT = "Yak, yak.";
char const *const SPECIAL_SCENE_HOTSPOT = "That is a unique use for that.";
char const *const DEFAULT_SCENE_HOTSPOT = "That accomplishes nothing.";
char const *const SAVE_ERROR_MSG = "Error occurred saving game. Please do not try to restore this game!";
char const *const SAVING_NOT_ALLOWED_MSG = "Saving is not allowed at this time.";
char const *const RESTORING_NOT_ALLOWED_MSG = "Restoring is not allowed at this time.";
char const *const INV_EMPTY_MSG = "You have nothing in your possesion.";

char const *const QUIT_CONFIRM_MSG = "Do you want to quit playing this game?";
char const *const RESTART_MSG = "Do you want to restart this game?";
char const *const GAME_PAUSED_MSG = "Game is paused.";
char const *const OK_BTN_STRING = " Ok ";
char const *const CANCEL_BTN_STRING = "Cancel";
char const *const QUIT_BTN_STRING = " Quit ";
char const *const RESTART_BTN_STRING = "Restart";
char const *const SAVE_BTN_STRING = "Save";
char const *const RESTORE_BTN_STRING = "Restore";
char const *const SOUND_BTN_STRING = "Sound";
char const *const RESUME_BTN_STRING = " Resume \rplay";
char const *const LOOK_BTN_STRING = "Look";
char const *const PICK_BTN_STRING = "Pick";


namespace Ringworld {

// Dialog resources
char const *const HELP_MSG = "Ringworld\rRevenge of the Patriarch\x14\rScummVM Version\r\r\
\x01 Keyboard shortcuts...\rF2 - Sound options\rF3 - Quit\r\
F4 - Restart\rF5 - Save game\rF7 - Restore Game\rF10 - Pause game";
char const *const WATCH_INTRO_MSG = "Do you wish to watch the introduction?";
char const *const START_PLAY_BTN_STRING = " Start Play ";
char const *const INTRODUCTION_BTN_STRING = "Introduction";
char const *const OPTIONS_MSG = "\x01Options...";

// Scene specific resources
char const *const EXIT_MSG = "   EXIT   ";
char const *const SCENE6100_CAREFUL = "Be careful! The probe cannot handle too much of that.";
char const *const SCENE6100_TOUGHER = "Hey! This is tougher than it looks!";
char const *const SCENE6100_ONE_MORE_HIT = "You had better be more careful. One more hit like that \
and the probe may be destroyed.";
char const *const SCENE6100_DOING_BEST = "I'm doing the best I can. I just hope it holds together!";
char const *const SCENE6100_REPAIR = "\r\rQuinn and Seeker repair the probe....";
char const *const SCENE6100_ROCKY_AREA = "The rocky area should be directly ahead of you. Do you see it?";
char const *const SCENE6100_REPLY = "Yes. Now if I can just avoid those sunbeams.";
char const *const SCENE6100_TAKE_CONTROLS = "You had better take the controls Seeker. My hands are sweating.";
char const *const SCENE6100_SURPRISE = "You surprise me Quinn. I would have thought you of hardier stock.";
char const *const SCENE6100_SWEAT = "Humans sweat, Kzin twitch their tail. What's the difference?";
char const *const SCENE6100_VERY_WELL = "Very well. I will retrieve the stasis box and return the probe. \
Wait for it's return in the lander bay.";

char const *const DEMO_HELP_MSG = " Help...\rF2 - Sound Options\rF3 - Exit demo\r\rPress ENTER\rto continue";
char const *const DEMO_PAUSED_MSG = "Ringworld\x14 demo is paused";
char const *const DEMO_EXIT_MSG = "Press ENTER to resume the Ringworld\x14 demo. Press ESC to exit";
char const *const EXIT_BTN_STRING = "Exit";
char const *const DEMO_BTN_STRING = "Demo";
char const *const DEMO_RESUME_BTN_STRING = "Resume";

} // End of namespace Ringworld

namespace BlueForce {

// Dialog resources
char const *const HELP_MSG = "Blue Force\x14\rScummVM Version\r\r\
Keyboard shortcuts...\rF2 - Sound options\rF3 - Quit\r\
F4 - Restart\rF5 - Save game\rF7 - Restore Game\rF10 - Pause game";
char const *const WATCH_INTRO_MSG = "Do you wish to watch the introduction?";
char const *const START_PLAY_BTN_STRING = " Play ";
char const *const INTRODUCTION_BTN_STRING = " Watch ";
char const *const OPTIONS_MSG = "Options...";

// Blue Force general messages
char const *const BF_NAME = "Blue Force";
char const *const BF_COPYRIGHT = " Copyright, 1993 Tsunami Media, Inc.";
char const *const BF_ALL_RIGHTS_RESERVED = "All Rights Reserved";
char const *const BF_19840518 = "May 18, 1984";
char const *const BF_19840515 = "May 15, 1984";
char const *const BF_3_DAYS = "Three days later";
char const *const BF_11_YEARS = "Eleven years later.";
char const *const BF_NEXT_DAY = "The Next Day";
char const *const BF_ACADEMY = "Here we are at the Academy";

// Scene 50 hotspots
char const *const GRANDMA_FRANNIE = "Grandma Frannie";
char const *const MARINA = "Marina";
char const *const POLICE_DEPARTMENT = "Police Department";
char const *const TONYS_BAR = "Tony's Bar";
char const *const CHILD_PROTECTIVE_SERVICES = "Child Protective Services";
char const *const ALLEY_CAT = "Alley Cat";
char const *const CITY_HALL_JAIL = "City Hall & Jail";
char const *const JAMISON_RYAN = "Jamison & Ryan";
char const *const BIKINI_HUT = "Bikini Hut";

// Scene 60 radio messages
char const *const RADIO_BTN_LIST[8] = { "10-2 ", "10-4 ", "10-13", "10-15", "10-27", "10-35", "10-97", "10-98" };

// Scene 570 computer messageS
char const *const SCENE570_PASSWORD = "PASSWORD -> ";
char const *const SCENE570_C_DRIVE = "C:\\";
char const *const SCENE570_RING = "RING";
char const *const SCENE570_PROTO = "PROTO";
char const *const SCENE570_WACKY = "WACKY";
char const *const SCENE570_COBB = "COBB";
char const *const SCENE570_LETTER = "LETTER";
char const *const SCENE570_RINGEXE = "RINGEXE";
char const *const SCENE570_RINGDATA = "RINGDATA";
char const *const SCENE570_PROTOEXE = "PROTOEXE";
char const *const SCENE570_PROTODATA = "PROTODATA";
char const *const SCENE570_WACKYEXE = "WACKYEXE";
char const *const SCENE570_WACKYDATA = "WACKYDATA";

// Scene 180 messages
char const *const THE_NEXT_DAY = "The Next Day";

} // End of namespace BlueForce

namespace Ringworld2 {

char const *const CONSOLE125_MESSAGES[] = {
	NULL, "Select Language", "Computer Services", "Food Services", "Entertainment Services",
	"Main Menu", "Exit Menu", "Previous Menu", "Interworld", "Hero's Tongue", "Personal Log",
	"Database", "Starchart", "Iso-Opto Disk Reader", "Eject Disk", "Meals", "Snacks",
	"Beverages", "Human Basic Snacks", "Kzin Basic Snacks", "Hot Beverages", "Cold Beverages",
	"Music", "Outpost Alpha", " ", " ", "Done", "A-G", "H-O", "P-S", "T-Z", "Tchaikovsky",
	"Mozart", "Bach", "Rossini"
};

char const *const CONSOLE325_MESSAGES[] = {
	NULL, "Select Language", "Database", "Star Chart", "Scan Ops", "Deep Scan",
	"Short Scan", "Main Menu", "Previous Menu", "Exit Menu", "Interworld", "Hero's Tongue",
	"Done", " ", " ", "Passive Enabled", "Active Enabled", "Technological", "Biological",
	"Geographical", "Astronomical", "Dipole Anomaly Sweep", "Structural Analysis",
	"A-G", "N-O", "P-S", "T-Z", "Tchaikovsky", "Mozart", "Bach", "Rossini"
};

// Scene 825 Autodoc messages
char const *const MAIN_MENU = "main menu";
char const *const DIAGNOSIS = "diagnosis";
char const *const ADVANCED_PROCEDURES = "advanced procedures";
char const *const VITAL_SIGNS = "vital signs";
char const *const OPEN_DOOR = "open door";
char const *const TREATMENTS = "treatments";
char const *const NO_MALADY_DETECTED =  "no malady detected";
char const *const NO_TREATMENT_REQUIRED = "no treatment required";
char const *const ACCESS_CODE_REQUIRED = "access code required";
char const *const INVALID_ACCESS_CODE = "invalid access code";
char const *const FOREIGN_OBJECT_EXTRACTED = "foreign object extracted";

char const *const HELP_MSG = "\x1\rRETURN TO\r RINGWORLD\x14";
char const *const CHAR_TITLE = "\x01Select Character:";
char const *const CHAR_QUINN_MSG = "  Quinn  ";
char const *const CHAR_SEEKER_MSG = "  Seeker  ";
char const *const CHAR_MIRANDA_MSG = "Miranda";
char const *const CHAR_CANCEL_MSG = "  Cancel  ";

char const *const GAME_VERSION = "ScummVM Version";
char const *const SOUND_OPTIONS = "Sound options";
char const *const QUIT_GAME = "Quit";
char const *const RESTART_GAME = "Restart";
char const *const SAVE_GAME = "Save game";
char const *const RESTORE_GAME = "Restore game";
char const *const SHOW_CREDITS = "Show credits";
char const *const PAUSE_GAME = "Pause game";
char const *const RESUME_PLAY = "  Resume play  ";
char const *const F2 = "F2";
char const *const F3 = "F3";
char const *const F4 = "F4";
char const *const F5 = "F5";
char const *const F7 = "F7";
char const *const F8 = "F8";
char const *const F10 = "F10";

} // End of namespace Ringworld2

} // End of namespace TsAGE
