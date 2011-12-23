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

#ifndef TSAGE_STATICRES_H
#define TSAGE_STATICRES_H

#include "common/scummsys.h"

namespace TsAGE {

extern const byte CURSOR_ARROW_DATA[];

extern const byte CURSOR_WALK_DATA[];

extern const char *LOOK_SCENE_HOTSPOT;
extern const char *USE_SCENE_HOTSPOT;
extern const char *TALK_SCENE_HOTSPOT;
extern const char *SPECIAL_SCENE_HOTSPOT;
extern const char *DEFAULT_SCENE_HOTSPOT;
extern const char *SAVE_ERROR_MSG;
extern const char *SAVING_NOT_ALLOWED_MSG;
extern const char *RESTORING_NOT_ALLOWED_MSG;

// Dialogs
extern const char *QUIT_CONFIRM_MSG;
extern const char *RESTART_MSG;
extern const char *GAME_PAUSED_MSG;
extern const char *OK_BTN_STRING;
extern const char *CANCEL_BTN_STRING;
extern const char *QUIT_BTN_STRING;
extern const char *RESTART_BTN_STRING;
extern const char *SAVE_BTN_STRING;
extern const char *RESTORE_BTN_STRING;
extern const char *SOUND_BTN_STRING;
extern const char *RESUME_BTN_STRING;
extern const char *LOOK_BTN_STRING;
extern const char *PICK_BTN_STRING;
extern const char *INV_EMPTY_MSG;

namespace Ringworld {

// Dialog resources
extern const char *HELP_MSG;
extern const char *WATCH_INTRO_MSG;
extern const char *START_PLAY_BTN_STRING;
extern const char *INTRODUCTION_BTN_STRING;
extern const char *OPTIONS_MSG;

// Scene specific resources
extern const char *EXIT_MSG;
extern const char *SCENE6100_CAREFUL;
extern const char *SCENE6100_TOUGHER;
extern const char *SCENE6100_ONE_MORE_HIT;
extern const char *SCENE6100_DOING_BEST;
extern const char *SCENE6100_REPAIR;
extern const char *SCENE6100_ROCKY_AREA;
extern const char *SCENE6100_REPLY;
extern const char *SCENE6100_TAKE_CONTROLS;
extern const char *SCENE6100_SURPRISE;
extern const char *SCENE6100_SWEAT;
extern const char *SCENE6100_VERY_WELL;

// Demo messages
extern const char *DEMO_HELP_MSG;
extern const char *DEMO_PAUSED_MSG;
extern const char *DEMO_HELP_MSG;
extern const char *DEMO_PAUSED_MSG;
extern const char *DEMO_EXIT_MSG;
extern const char *EXIT_BTN_STRING;
extern const char *DEMO_BTN_STRING;
extern const char *DEMO_RESUME_BTN_STRING;

} // End of namespace Ringworld

namespace BlueForce {

// Dialog resources
extern const char *HELP_MSG;
extern const char *WATCH_INTRO_MSG;
extern const char *START_PLAY_BTN_STRING;
extern const char *INTRODUCTION_BTN_STRING;
extern const char *OPTIONS_MSG;

// Blue Force messages
extern const char *BF_NAME;
extern const char *BF_COPYRIGHT;
extern const char *BF_ALL_RIGHTS_RESERVED;
extern const char *BF_19840518;
extern const char *BF_19840515;
extern const char *BF_3_DAYS;
extern const char *BF_11_YEARS;
extern const char *BF_NEXT_DAY;
extern const char *BF_ACADEMY;

// Scene 50 tooltips
extern const char *GRANDMA_FRANNIE;
extern const char *MARINA;
extern const char *POLICE_DEPARTMENT;
extern const char *TONYS_BAR;
extern const char *CHILD_PROTECTIVE_SERVICES;
extern const char *ALLEY_CAT;
extern const char *CITY_HALL_JAIL;
extern const char *JAMISON_RYAN;
extern const char *BIKINI_HUT;

extern const char *SCENE570_PASSWORD;
extern const char *SCENE570_C_DRIVE;
extern const char *SCENE570_RING;
extern const char *SCENE570_PROTO;
extern const char *SCENE570_WACKY;
extern const char *SCENE570_COBB;
extern const char *SCENE570_LETTER;
extern const char *SCENE570_RINGEXE;
extern const char *SCENE570_RINGDATA;
extern const char *SCENE570_PROTOEXE;
extern const char *SCENE570_PROTODATA;
extern const char *SCENE570_WACKYEXE;
extern const char *SCENE570_WACKYDATA;

// Scene 60 radio dispatch buttons
extern const char *RADIO_BTN_LIST[8];

// Scene 180 message
extern const char *THE_NEXT_DAY;

} // End of namespace BlueForce

namespace Ringworld2 {

// Scene 125 - Console messages
extern const char *CONSOLE_MESSAGES[];

// Scene 825 - Autodoc Messages
extern const char *MAIN_MENU;
extern const char *DIAGNOSIS;
extern const char *ADVANCED_PROCEDURES;
extern const char *VITAL_SIGNS;
extern const char *OPEN_DOOR;
extern const char *TREATMENTS;
extern const char *NO_MALADY_DETECTED;
extern const char *NO_TREATMENT_REQUIRED;
extern const char *ACCESS_CODE_REQUIRED;
extern const char *INVALID_ACCESS_CODE;
extern const char *FOREIGN_OBJECT_EXTRACTED;
extern const char *AUTODOC_ITEMS[11];

// Dialog messages
extern const char *HELP_MSG;
extern const char *CHAR_TITLE;
extern const char *CHAR_QUINN_MSG;
extern const char *CHAR_SEEKER_MSG;
extern const char *CHAR_MIRANDA_MSG;
extern const char *CHAR_CANCEL_MSG;

extern const char *GAME_VERSION;
extern const char *SOUND_OPTIONS;
extern const char *QUIT_GAME;
extern const char *RESTART_GAME;
extern const char *SAVE_GAME;
extern const char *RESTORE_GAME;
extern const char *SHOW_CREDITS;
extern const char *PAUSE_GAME;
extern const char *RESUME_PLAY;
extern const char *F2;
extern const char *F3;
extern const char *F4;
extern const char *F5;
extern const char *F7;
extern const char *F8;
extern const char *F10;

} // End of namespace Ringworld2

} // End of namespace TsAGE

#endif
