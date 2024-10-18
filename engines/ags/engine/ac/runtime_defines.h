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

#ifndef AGS_ENGINE_AC_RUNTIME_DEFINES_H
#define AGS_ENGINE_AC_RUNTIME_DEFINES_H

#include "ags/shared/ac/common_defines.h"

namespace AGS3 {

// xalleg.h pulls in an Allegro-internal definition of MAX_TIMERS which
// conflicts with the definition in runtime_defines.h. Forget it.
#ifdef MAX_TIMERS
#undef MAX_TIMERS
#endif

// Max old-style script string length
#define MAX_MAXSTRLEN 200
#define MAXGLOBALVARS 50

#define INVALID_X  30000
#define MAXGSVALUES 500
#define MAXGLOBALSTRINGS 51
#define MAX_INVORDER 500
#define DIALOG_NONE      0
#define DIALOG_RUNNING   1
#define DIALOG_STOP      2
#define DIALOG_NEWROOM   100
#define DIALOG_NEWTOPIC  12000
#define MAX_TIMERS       21
#define MAX_PARSED_WORDS 15
// how many saves may be listed at once
#define MAXSAVEGAMES     50
// topmost save index to be listed with a FillSaveGameList command
// NOTE: changing this may theoretically affect older games which
// use slots > 99 for special purposes!
#define TOP_LISTEDSAVESLOT 99
#define MAX_QUEUED_MUSIC 10
#define GLED_INTERACTION 1
#define GLED_EFFECTS     2
#define QUEUED_MUSIC_REPEAT 10000
#define MAX_AUDIO_TYPES  30

// Legacy (pre 3.5.0) alignment types used in the script API
enum LegacyScriptAlignment {
	kLegacyScAlignLeft = 1,
	kLegacyScAlignCentre = 2,
	kLegacyScAlignRight = 3
};

const int LegacyMusicMasterVolumeAdjustment = 60;
const int LegacyRoomVolumeFactor = 30;

// Common command arguments
// HISTORICAL NOTE: These numbers were chosen arbitrarily -- the idea is
// to make sure that the user gets the parameters the right way round
// Walk (pathfinding) modes
#define ANYWHERE       304
#define WALKABLE_AREAS 305
// Blocking / non-blocking action
#define BLOCKING       919
#define IN_BACKGROUND  920
// Direction of animation
#define FORWARDS       1062
#define BACKWARDS      1063
// Stop / don't stop when changing a view
#define STOP_MOVING    1
#define KEEP_MOVING    0

#define SCR_NO_VALUE   31998
#define SCR_COLOR_TRANSPARENT -1


#define TXT_SCOREBAR        29
#define MAXSCORE _GP(play).totalscore

#define FONT_STATUSBAR  0
#define FONT_NORMAL     _GP(play).normal_font
//#define FONT_SPEECHBACK 1
#define FONT_SPEECH     _GP(play).speech_font

// Standard interaction verbs (aka cursor modes)
#define MODE_NONE	   -1
#define MODE_WALK		0
#define MODE_LOOK		1
#define MODE_HAND		2
#define MODE_TALK		3
#define MODE_USE		4
#define MODE_PICKUP 	5
// aka MODE_POINTER
#define CURS_ARROW		6
// aka MODE_WAIT
#define CURS_WAIT		7
#define MODE_CUSTOM1	8
#define MODE_CUSTOM2	9
#define NUM_STANDARD_VERBS 10

// Fixed Overlay IDs
#define OVER_TEXTMSG  1
#define OVER_COMPLETE 2
#define OVER_PICTURE  3
#define OVER_TEXTSPEECH 4
#define OVER_FIRSTFREE 5
#define OVER_CUSTOM   -1
// Overlay parameters
#define OVR_AUTOPLACE 30000

#define FOR_ANIMATION 1
#define FOR_SCRIPT    2
#define FOR_EXITLOOP  3

// an actsps index offset for characters
#define ACTSP_OBJSOFF (MAX_ROOM_OBJECTS)
// a 1-based movelist index offset for characters
#define CHMLSOFFS (1 + MAX_ROOM_OBJECTS)
#define MAX_SCRIPT_AT_ONCE 10
#define EVENT_NONE       0
#define EVENT_INPROGRESS 1
#define EVENT_CLAIMED    2

// Internal skip style flags, for speech/display, wait;
 // theoretically correspond to InputType in script (with a 24-bit shift)
#define SKIP_NONE       0x00
#define SKIP_AUTOTIMER  0x01
#define SKIP_KEYPRESS   0x02
#define SKIP_MOUSECLICK 0x04
// Bit shift for packing skip type into result
#define SKIP_RESULT_TYPE_SHIFT 24
// Bit mask for packing skip key/button data into result
#define SKIP_RESULT_DATA_MASK  0x00FFFFFF

// The index base for characters, used in legacy AnimateObject script function;
// if passed ID is eq or gt than this, then a Character is animated instead
#define LEGACY_ANIMATE_CHARIDBASE 100

#define STD_BUFFER_SIZE 3000

// NOTE: these flags are merged with the MoveList index;
// but this means that the number of MoveList users will be limited by 1000
#define TURNING_AROUND     1000
#define TURNING_BACKWARDS 10000

#define LOCTYPE_HOTSPOT 1
#define LOCTYPE_CHAR 2
#define LOCTYPE_OBJ  3

#define MAX_DYNAMIC_SURFACES 20

#define RESTART_POINT_SAVE_GAME_NUMBER 999

#define MAX_OPEN_SCRIPT_FILES 10

#define RETURN_CONTINUE 1

} // namespace AGS3

#endif
